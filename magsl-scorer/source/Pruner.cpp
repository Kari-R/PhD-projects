#include <Pruner.hpp>

double Pruner::get_max_sub_score(Pruneable& p) {
    double best = Scoreable::MinScore;
    for(node_t x = 0; x < p.node_count(); x++) {
        for(node_t y = 0; y < bits_t().size(); y++) {
            if(p.has_spouse(x, y)) {
                p.remove_spouse(x, y);
                best = std::max(best, get_max_score(p));
                p.add_spouse(x, y);
            }
        }
        for(node_t y = 0; y < bits_t().size(); y++)
            if(p.has_parent(x, y)) {
                p.remove_parent(x, y);
                best = std::max(best, get_max_score(p));
                p.add_parent(x, y);
            }
    }
    return best;
}

void gather_partitions(std::vector<bits_t>& partitions, bits_t subset, bits_t vars, node_t i = 0) {
    if(i >= vars.size()) {
        partitions.push_back(subset);
        return;
    }
    gather_partitions(partitions, subset, vars, i + 1);
    if(vars.test(i))
        gather_partitions(partitions, subset.set(i), vars, i + 1);
}

double Pruner::get_max_partition_score(Pruneable& target) {
    static std::vector<bits_t> partitions[8];
    
    node_t count = target.node_count();
    size_t full_set = (size_t(1) << count) - 1;
    if(count < 2)
        return Scoreable::MinScore;
    else if(partitions[count].empty())
        gather_partitions(partitions[count], 0, full_set);

    double max = Scoreable::MinScore;
    for(bits_t subset: partitions[count]) {
        if(subset.count() == 0 || subset.count() >= target.node_count())
            continue;
        
        Pruneable partition1(target, subset);
        Pruneable partition2(target, bits_t(full_set) &~ subset);

        double s1 = get_max_score(partition1);
        double s2 = get_max_score(partition2);
        if(s1 != Scoreable::MinScore && s2 != Scoreable::MinScore)
            max = std::max(max, s1 + s2);
    }
    return max;
}

double Pruner::get_max_score(Pruneable& target) {
    auto it = cache.find(target);
    if(it == cache.end())
        return Scoreable::MinScore;
    else if(it->second.is_processed)
        return it->second.max_score;

    it->second.max_score = std::max(it->second.max_score, get_max_sub_score(target));    
    if(prune_level == 2)
        it->second.max_score = std::max(it->second.max_score,
            get_max_partition_score(target));
    it->second.is_processed = true;
    return it->second.max_score;
}

std::pair<size_t, size_t> Pruner::optima_rate_after_pruning() const {
    size_t multiple_optimas = 0, most = 1;
    for(auto it = cache.begin(); it != cache.end(); it++) {
        auto it2 = optima_count.find(it->first);
        if(it2 != optima_count.end()) {
            if(it2->second > 1)
                multiple_optimas++;
            if(most < it2->second)
                most = it2->second;
        }
    }
    return {multiple_optimas, most};
}

void Pruner::prune() {
    if(!prune_level)
        return;

    size_t at_first = scores.size();
    size_t start_time = get_time();
    print("  Pruning scores...");
    for(const ScoredItem& item: scores) {
        double starting_score = item.first.usable_for_pruning?
            item.second: Scoreable::MinScore;
        cache.insert({ Pruneable(item.first), {starting_score, false} });
    }

    for(auto it = scores.begin(); it != scores.end();) {
        Pruneable pruneable(it->first);
        if(it->second <= get_max_sub_score(pruneable) || (prune_level == 2 &&
                it->second <= get_max_partition_score(pruneable))) {
            auto it2 = optima_count.find(it->first);
            if(it2 != optima_count.end())
                optima_count.erase(it2);
            it = scores.erase(it);
        } else
            it++;
    }
    print(" Done.\n   -> % scores (%% got pruned)\n",
        scores.size(), percentage(at_first - scores.size(), at_first), '%');
    prune_count += at_first - scores.size();
    prune_time += get_time() - start_time;
}

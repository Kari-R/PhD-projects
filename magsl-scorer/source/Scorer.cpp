#include <Scorer.hpp>
#include <ScoreSearch.hpp>
#include <iomanip>
#include <map>

Scorer::Scorer(const Config& config,
        ICF* scorer, size_t max_pars_per_node, size_t max_pars_per_comp, std::ostream& out, size_t max_decimals ):
    config(config), scorer(scorer),
    max_pars_per_node(max_pars_per_node),
    max_pars_per_comp(max_pars_per_comp),
    out(out), max_decimals(max_decimals), pruner(config.get(PruneLevel)), total_count(0) {}

void Scorer::iterate_extensions(Scoreable target, bits_t component) {
    ensure(target.nodes.empty());
    ensure(target.outer_parents.empty());
    ensure(component.count() == target.parents.size());

    std::vector<node_t> remain;
    for(node_t i = 0; i < scorer->variable_count(); i++)
        if(component.test(i))
            target.nodes.push_back(i);
        else
            remain.push_back(i);
    size_t parent_count = 0;
    for(node_t i = 0; i < target.component_size(); i++)
        parent_count += target.parents[i].size();
    iterate_extensions(target, remain, 0, 0, parent_count);
}

bool Scorer::output_missing_edges(const Scoreable& target) const {
    bool has_missing = false;
    for(node_t x = 0; x < target.component_size(); x++) {
        bits_t edges;
        for(node_t sp: target.spouses[x])
            edges.set(sp);
        for(node_t y = x + 1; y < target.component_size(); y++)
            if(!edges.test(y)) {
                out << " " << bits_t().set(target.nodes[x]).set(target.nodes[y]).to_ulong();
                has_missing = true;
            }
    }
    return has_missing;
}

void Scorer::output_parentsets(const Scoreable& target) const {
    for(node_t i = 0; i < target.component_size(); i++) {
        size_t bits = 0;
        for(node_t j: target.parents[i])
            bits |= size_t(1) << target.nodes[j];
        out << " " << bits;
    }
}

void Scorer::prune_and_output() {
    if(config.get(PruneLevel) != 0)
        pruner.prune();
    auto pruned = pruner.get();
    for(const ScoredItem& item: pruned)
        output(item.first, item.second);
    pruner.clear();
}

void Scorer::output(const Scoreable& target, double score) const {
    out << std::setprecision(max_decimals) << score << " ";
    size_t component = 0;
    for(node_t i = 0; i < target.component_size(); i++)
        component |= size_t(1) << target.nodes[i];
    out << component;
    output_parentsets(target);
    if(!output_missing_edges(target))
        out << " 0";
    out << std::endl;
}

void Scorer::consider(const Scoreable& target) const {
    total_count++;
    size_t component = 0;
    for(node_t i = 0; i < target.component_size(); i++)
        component |= size_t(1) << target.nodes[i];
    double score;
    if(config.get(ICF_Repetitions) == 1 && is_complete(target)) {
        auto it = complete_scores.find(component);
        if(it == complete_scores.end())
            it = complete_scores.insert(complete_scores.begin(),
                std::make_pair(component, scorer->score(target)));
        score = it->second;
    } else {
        auto stats = scorer->score(target, config.get(ICF_Repetitions), config.get(ICF_Max_Optimas));
        score = stats.highest;
        pruner.mark_optima_count(target, stats.local_optimas);
    }
    pruner.add(target, score);
}

bits_t get_outer_parents(const Scoreable& target, size_t node) {
    bits_t parents;
    for(node_t parent: target.parents[node])
        if(parent >= target.component_size())
            parents.set(target.nodes[parent]);
    return parents;
}

bool Scorer::is_complete(const Scoreable& target) const {
    if(!target.is_complete)
        return false;
    for(node_t node = 0; node < target.component_size(); node++)
        if(get_outer_parents(target, node).any())
            return false;
    return true;
}

void Scorer::iterate_extensions(const Scoreable& target, const Nodes& parents, node_t node_id, node_t parent_id, size_t parent_count) {  
    if(parent_id >= parents.size())
        consider(target);
    else if(node_id >= target.component_size())
        iterate_extensions(target, parents, 0, parent_id + 1, parent_count);
    else {
        iterate_extensions(target, parents, node_id + 1, parent_id, parent_count);

        if(target.parents[node_id].size() < max_pars_per_node &&
                parent_count < max_pars_per_comp) {
            Scoreable extended(target);
            extended.add_outer_parent(node_id, parents[parent_id]);
            iterate_extensions(extended, parents, node_id + 1, parent_id, parent_count + 1);
        }
    }
}

size_t subset_count(size_t n, size_t k_max) {
    size_t count = 0;
    for(size_t k = 0; k <= k_max; k++)
        count += binomial(n, k);
    return count;
}

size_t Scorer::output_total_score_count(size_t max_comp) const {
    size_t total = 0;
    std::vector<size_t> counts;
    std::vector<Scoreables> scoreables;
    for(size_t comp_size = 1; comp_size <= max_comp; comp_size++) {
        counts.push_back(0);
        scoreables.push_back(ScoreSearch(comp_size, max_pars_per_node, max_pars_per_comp).search());
        for(const Scoreable& target: scoreables.back()) {
            size_t combos = binomial(scorer->variable_count(), comp_size);
            for(size_t i = 0; i < comp_size; i++)
                combos *= subset_count(scorer->variable_count() - comp_size,
                    std::min(scorer->variable_count() - comp_size, max_pars_per_node - target.parents[i].size()));
            counts.back() += combos;
        }
        total += counts.back();
    }
    print("Expecting % scores before pruning:\n", total);
    for(size_t i = 0; i < counts.size(); i++)
        print("  %-node c-components: % scores (%\\% of total), % patterns\n",
            i + 1, counts[i], percentage(counts[i], total), scoreables[i].size());
    return total;
}

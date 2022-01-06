#include <DomainBuilder.hpp>

bool OptionsSort::operator()(size_t a, size_t b) const {

    if(calculate_ub) {
        Score s1 = (* calculate_ub)(a / domains[0].size(), a % domains[0].size());
        Score s2 = (* calculate_ub)(b / domains[0].size(), b % domains[0].size());
        
        if(s1 != s2)
            return s1 > s2;
    } else {
        Score s1 = domains[a / domains[0].size()][a % domains[0].size()].score;
        Score s2 = domains[b / domains[0].size()][b % domains[0].size()].score;

        if(s1 != s2)
            return s1 > s2;
    }
    return a < b;
}

DomainBuilder::DomainBuilder(
Instance& i,
Solution& s,
ValueCalculator * ubc,
uint mv):
    instance(i),
    state(s),
    option_list(OptionsSort { ubc, instance.domains }),
    calculate_ub(ubc),
    N(mv) {
    
    latest_var = 0;
    for(uint v = 1; v < N; v++)
        if(state.contains(v) && (!state.contains(latest_var) ||
                state[latest_var].depth <= state[v].depth))
            latest_var = v;
}

#define parents_of(var) \
    instance.domains[var][state[var].parentset]

Score DomainBuilder::build_supersets(uint var, std::bitset<32> subset) {
    Score maximum = -INF;
    
    for(uint added = 0; added < N; added++) {
        if(subset[added] || added == var || !state.contains(added))
            continue;

        if(subset != (subset & state.skeleton[added]))
            continue;
        
        subset[added] = true;
        Score superset_maximum = build(var, subset);
        if(maximum < superset_maximum)
            maximum = superset_maximum;
        subset[added] = false;
    }
    return maximum;
}

bool DomainBuilder::in_same_subgraph(std::bitset<32>& visited, uint vertex, uint target) {
    if(visited.test(vertex)) return false;
    if(vertex == target) return true;
    visited.set(vertex);
    for(uint parent: instance.domains[vertex][state[vertex].parentset].list)
        if(in_same_subgraph(visited, parent, target))
            return true;
    return false;
}

bool DomainBuilder::worsens_lexicographicality(uint var, const ParentSet& parentset) {
    for(uint v = var + 1; v < N; v++) {
        if(!state.contains(v))
            continue;

        auto content = instance.domains[v][state[v].parentset].content;
        if(content == (content & parentset.content)) {
            std::bitset<32> visited;
            for(uint parent: parentset.list)
                if(in_same_subgraph(visited, parent, v))
                    return true;
        }
    }
    return false;
}

Score DomainBuilder::build(uint var, const std::bitset<32>& subset) {

    auto it = superset_scores.find(subset);
    if(it != superset_scores.end())
        return it->second;        

    size_t pset = instance.domain_lookup[var].at(subset);
    if(worsens_lexicographicality(var, instance.domains[var][pset]))
        return -INF;
    
    Score maximum = subset.count() >= instance.max_pset? -INF:
        build_supersets(var, subset);

    if(maximum < instance.domains[var][pset].score) {
        maximum = instance.domains[var][pset].score;
        option_list.insert(pset + var * instance.domains[0].size());
    }

    superset_scores.insert(std::make_pair(subset, maximum));
    return maximum;
}

void DomainBuilder::construct_for(uint var) {
    superset_scores.clear();
    uint min_depth = state[latest_var].depth + (var < latest_var);
    
    if(state.assign_mask.none() || !min_depth)
        build(var, std::bitset<32>());
    else {
        for(uint v = 0; v < N; v++)
            if(state.contains(v) && state[v].depth + 1 >= min_depth) {
                std::bitset<32> subset;
                subset.set(v);
                build(var, subset);
            }
    }
}

uint DomainBuilder::get_depth(const Solution& state, const ParentSet& parentset) {
    uint depth = 0;
    for(uint parent: parentset.list)
        if(depth < state[parent].depth + 1)
            depth = state[parent].depth + 1;
    return depth;
}
#pragma once

#include <ICF.hpp>
#include <Graph.hpp>

class ScoreSearch {
public:
    ScoreSearch(size_t max_comp, size_t max_parents = 1000, size_t max_pars_by_comp = 1000000):
        comp_size(max_comp), max_pars_per_node(max_parents), max_pars_per_comp(max_pars_by_comp) {}

    inline const std::vector<Scoreable>& search() {
        iterate_spouses(Graph(comp_size), 0, 1);
        return targets;
    }
    
private:
    void add_target(const Graph&);

    void iterate_parents(const Graph&, node_t, size_t, size_t);
    void iterate_spouses(const Graph&, node_t, node_t);

private:
    std::vector<Scoreable> targets;
    size_t comp_size, max_pars_per_node, max_pars_per_comp;
};

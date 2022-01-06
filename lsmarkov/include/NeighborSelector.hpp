#pragma once

#include <Graph.hpp>

enum Operations {
    VERTEX_OP = 'V',
    EDGE_OP = 'E',
    CLIQUE_OP = 'C',
    OP_COUNT = 4
};

class NeighborSelector {
public:
    static size_t& solution_count() { static size_t c = 0; return c; }
    
    static size_t& usecount(size_t oper_id) {
        static std::vector<size_t> counts(128, 0);
        return counts[oper_id];
    }
    
    NeighborSelector(const Graph& g):
        instance(*g.instance), graph(g), best(*g.instance) {
            best.init_tree();
    }

    void consider_edge_based(var_t, var_t start = 0);
    
    void apply_change(Graph&, const Bitset&, var_t);
    
    void consider_clique_based(const Bitset&);
    void consider_variable_based(const std::vector<Bitset>&, var_t);
    
    const Graph& consider();
    
    inline void consider(const Graph& neighbor, size_t oper) {
        if(best.score() < neighbor.score()) {
            best = neighbor;
            best_oper = oper;
        }
        NeighborSelector::solution_count()++;
    }
    
private:
    const Instance& instance;
    const Graph& graph;
    Graph best;
    size_t best_oper = OP_COUNT;
};

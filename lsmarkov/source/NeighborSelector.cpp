#include <NeighborSelector.hpp>

void NeighborSelector::consider_edge_based(var_t var, var_t start) {
    for(var_t v = start; v < graph.size(); v++) {
        if(var == v)
            continue;
        Graph neighbor(graph);
        if(neighbor.contains_edge(var, v))
            neighbor.remove_edge(var, v);
        else
            neighbor.add_edge(var, v);
        consider(neighbor, EDGE_OP);
    }
}

void NeighborSelector::apply_change(Graph& neighbor, const Bitset& clique, var_t var) {
    if(clique.test(var)) {
        for(var_t v: clique.contents())
            if(var != v)
                neighbor.remove_edge(var, v);
    } else
        for(var_t v: clique.contents())
            neighbor.add_edge(var, v);
}

void NeighborSelector::consider_clique_based(const Bitset& clique) {
    for(var_t var = 0; var < graph.size(); var++) {
        Graph neighbor(graph);
        apply_change(neighbor, clique, var);
        consider(neighbor, CLIQUE_OP);
    }
}

void NeighborSelector::consider_variable_based(const std::vector<Bitset>& cliques, var_t var) {
    for(const Bitset& clique: cliques) {
        Graph neighbor(graph);
        apply_change(neighbor, clique, var);
        consider(neighbor, VERTEX_OP);
    }
}

const Graph& NeighborSelector::consider() {
    std::vector<Bitset> cliques;
    graph.extract_cliques(cliques);

    if(instance[GREEDY_BASELINE].on()) {
        for(var_t var = 0; var < graph.size(); var++) {
            if(instance[OPERATIONS].contains(EDGE_OP))
                consider_edge_based(var, var + 1);
            if(instance[OPERATIONS].contains(VERTEX_OP))
                consider_variable_based(cliques, var);
        }
        if(instance[OPERATIONS].contains(CLIQUE_OP))
            for(const Bitset& clique: cliques)
                consider_clique_based(clique);
    } else {
        if(instance[OPERATIONS].contains(EDGE_OP))
            consider_edge_based(Rand::get(0, graph.size()));

        if(instance[OPERATIONS].contains(VERTEX_OP))
            consider_variable_based(cliques, Rand::get(0, graph.size()));

        if(instance[OPERATIONS].contains(CLIQUE_OP))
            consider_clique_based(cliques[ Rand::get(0, cliques.size()) ]);
    }

    if(best_oper != OP_COUNT)
        usecount(best_oper)++;
    return best;
}

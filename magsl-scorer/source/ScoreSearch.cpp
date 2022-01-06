#include <ScoreSearch.hpp>

void ScoreSearch::add_target(const Graph& graph) {
    targets.push_back( Scoreable(comp_size) );
    targets.back().is_complete = graph.is_complete();
    targets.back().usable_for_pruning = !graph.has_bad_indpath();

    for(node_t x = 0; x < graph.size(); x++) {
        for(node_t y = x + 1; y < graph.size(); y++)
            if(graph.has_bidirected_edge(x, y))
                targets.back().add_bidir_edge(x, y);
            else if(graph.has_edge(x, y))
                targets.back().add_inner_parent(y, x);
            else if(graph.has_edge(y, x))
                targets.back().add_inner_parent(x, y);
    }
    targets.back().sort_spouses();
}

void ScoreSearch::iterate_parents(const Graph& graph, node_t node, size_t nodes_parents, size_t components_parents) {
    if(graph.has_cycle())
        return;
    else if(node >= graph.size()) {
        if(graph.size() < 4 || graph.is_maximal())
            add_target(graph);
    } else {
        if(nodes_parents < max_pars_per_node && components_parents < max_pars_per_comp)
            for(node_t parent = 0; parent < graph.size(); parent++) {
                if(node == parent || graph.has_bidirected_edge(node, parent) ||
                        graph.has_edge(node, parent) || graph.has_edge(parent, node))
                    continue;

                Graph extended(graph);
                extended.add_parent(node, parent);
                iterate_parents(extended, node, nodes_parents + 1, components_parents + 1);
            }
        iterate_parents(graph, node + 1, 0, components_parents);
    }
}

void ScoreSearch::iterate_spouses(const Graph& graph, node_t node, node_t first_spouse) {
    if(node >= graph.size()) {
        if(!graph.is_strongly_connected())
            return;
        iterate_parents(graph, 0, 0, 0);
    } else {
        for(node_t spouse = first_spouse; spouse < graph.size(); spouse++) {
            ensure(!graph.has_bidirected_edge(node, spouse));
            Graph extended(graph);
            extended.add_bidirected(node, spouse);
            iterate_spouses(extended, node, spouse + 1);
        }
        iterate_spouses(graph, node + 1, node + 2);
    }
}

#include <Search.hpp>
#include <NeighborSelector.hpp>

size_t get_width(const Graph& graph) {
    std::vector<Bitset> cliques;
    graph.extract_cliques(cliques);
    size_t max = 0;
    for(const Bitset& clique: cliques)
        if(max < clique.count())
            max = clique.count();
    return max;
}

void Search::update_score(Graph& graph, size_t iters, bool force_update) {

    if(force_update || best.score() < graph.score()) {
        std::string prev_score = format("%", best.score());
        best = graph;
        std::string str_score = format("%", best.score());
        if(force_update || prev_score != str_score) {
            double p_v = NeighborSelector::usecount(VERTEX_OP);
            double p_e = NeighborSelector::usecount(EDGE_OP);
            double p_c = NeighborSelector::usecount(CLIQUE_OP);

            #define PERCENT(x) p_v + p_e + p_c == 0? 0: int(x/(p_v + p_e + p_c)*100)
 
            print("update % % % % % % % % % %\n",
                justify(best.score(), 16),
                justify((get_time() - start_time) / 1000000, 5),
                justify(instance.score_comp_time() / 1000000, 5),
                justify(restarts, 6),
                justify(iters, 10),
                justify(PERCENT(p_v), 3),
                justify(PERCENT(p_e), 3),
                justify(PERCENT(p_c), 3),
                justify(get_width(best), 3),
                justify(instance.computed_clique_scores(), 8));
            print("solution %\n\n", best);
        }
    }
}

void Search::search() {
    if(instance[DYNAMIC_TW].on())
        instance.reset_dynamic_treewidth();
    
    Graph graph(instance);
    graph = best;
    
    if(instance[USE_CHOW_LIU].on()) {
        graph.init_tree();
        print("Chow-Liu solution: %  (Score: %)\n", graph, graph.score());
    }
    
    print("\n       % % % % % % % % % %\n",
        justify("score", 16),
        justify("time", 5),
        justify("clsc", 5),
        justify("starts", 6),
        justify("iters", 10),
        justify("V%", 3),
        justify("E%", 3),
        justify("C%", 3),
        justify("cs", 3),
        justify("scoreuse", 8));
    
    Graph dynamic_best = graph;
    
    local_best = graph.score();
    iters_to_best = 0;
    restarts = 0;
    update_score(graph, 0, true);

    for(size_t iters = 1;; iters++) {
        NeighborSelector selector(graph);
        graph = selector.consider();

        if(local_best < graph.score()) {
            local_best = graph.score();
            dynamic_best = graph;

            if(!instance[FIXED_ITERS].on())
                iters_to_best = iters;
        } else if(instance[GREEDY_BASELINE].on()) {
            graph.init_rand();
            local_best = graph.score();
            restarts++;
            continue;
        }
        
        update_score(graph, iters);
        
        if(iters >= iters_to_best + instance[RESTART_RATE].number && !instance[GREEDY_BASELINE].on()) {
            if(instance[DYNAMIC_TW].on()) {
                if(!instance.increase_dynamic_treewidth()) {
                    instance.reset_dynamic_treewidth();
                    graph.init_rand();
                    dynamic_best = graph;
                    restarts++;
                } else
                    graph = dynamic_best;
                local_best = graph.score();
            } else {
                graph.init_rand();
                local_best = graph.score();
                restarts++;
            }
            iters_to_best = iters;
        }
    }
}

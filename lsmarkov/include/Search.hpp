#pragma once

#include <Graph.hpp>

class Search {
public:
    Search(Instance& inst, size_t stime):
        instance(inst), best(inst), start_time(stime) {
            if(instance[USE_CHOW_LIU].on())
                best.init_tree();
            else
                best.init_rand();
        }

    void search();
    void update_score(Graph&, size_t, bool force_update = false);

private:
    Instance& instance;
    Graph best;
    size_t start_time, iters_to_best, restarts;
    score_t local_best;
};

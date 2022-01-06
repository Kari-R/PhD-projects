#include <DPSolver.hpp>

const Solution& DPSolver::solve(const Solution& base_solution, const Reach& reach, bits_t remain) {
    bool first_call = !iterations;
    result = base_solution;
    score_t score = solve(reach, remain, true);
    if(first_call)
        print("DP-Score: %\n", -score);
    return result;
}

#define IF_CACHE(...) __VA_ARGS__

score_t DPSolver::solve(const Reach& reach, bits_t remain, bool printing) {
    if(remain.none())
        return 0;

    IF_CACHE(
        Reach::Key point(remain, reach, config.has(EXPERIMENTAL));
        auto it = cache.find(point);
        if(it != cache.end() && !printing) {
            cache_hits++;
            return it->second;
        }
        it = cache.insert(cache.begin(),
            std::make_pair(point, MAX_SCORE));
    );
    iterations++;
    
    node_t smallest_node = 0;
    while(!remain.test(smallest_node))
        smallest_node++;

    std::vector<bits_t> comps;
    if(find_BN)
	comps.push_back( bits_t().set(smallest_node) );
    else
    	instance.gather_subsets(comps, bits_t().set(smallest_node), remain, smallest_node + 1);

    score_t best = MAX_SCORE;
    size_t best_id = -1;
    for(bits_t comp: comps) {
        const std::vector<size_t>& choices = instance.by_component(comp);

        for(size_t id: choices) {
            const Comparent& scoreable = instance.get(id);
            ensure(scoreable.component().bitset() == comp);

            Reach new_reach(reach);
            scoreable.update_reach(new_reach);

            if(new_reach.has_cycle())
                continue;

            new_reach.simplify_past(remain, config.has(SIMPLIFY));
            
            score_t score = scoreable.score() + solve(new_reach, remain &~ comp, false);
            if(best > score) {
                best = score;
                best_id = id;
            }
        }
    }

    if(printing) {
        const Comparent& scoreable = instance.get(best_id);
        result.add(scoreable);

        Reach new_reach(reach);
        scoreable.update_reach(new_reach);

        ensure(!new_reach.has_cycle());
        ensure(!result.has_cycle());

        new_reach.simplify_past(remain, config.has(SIMPLIFY));
        solve(new_reach, remain &~ scoreable.component().bitset(), true);
    }

    IF_CACHE( it->second = best; )
    return best;
}

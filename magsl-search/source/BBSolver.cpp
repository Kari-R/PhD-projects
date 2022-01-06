#include <BBSolver.hpp>

BBSolver::BBSolver(const Config& config, const Scores& scores):
    config(config), instance(scores), dp(config, scores), best_solution(scores.size(), MAX_SCORE) {};

void BBSolver::solve(size_t time, const Solution* start_solution) {
    start_time = time;
    iterations = 0;
    nonmaximals = 0;
    best_solution = start_solution != nullptr? *start_solution: Solution(instance.size(), MAX_SCORE);
    Reach reach(instance.size());
    solve(reach, filled_bitset<bits_t>(instance.size()), Solution(instance.size(), 0));
    print("\n========================= Optimal Solution Found\n");
    best_solution.output();
    print("\nIterations: %\nNon-maximal solutions encountered: %\n", iterations, nonmaximals);
}

void BBSolver::solve(const Reach& reach, bits_t remain, const Solution& solution) {
    iterations++;
    
    if(!solution.is_maximal(remain)) {
        nonmaximals++;
        return;
    }
    
    if(remain.none()) {
        if(best_solution.score() > solution.score()) {            
            print("%s Update: %  [at leaf node]\n", duration_since(start_time), -solution.score());
            best_solution = solution;
        }
        return;
    }

    Solution lb_solution = dp.solve(solution, reach, remain);
    if(best_solution.score() <= lb_solution.score())
        return;

    if(config.has(CHECK_LB_MAXIMALITY) && lb_solution.is_maximal()) {
        print("%s Update: %  [maximal UB solution]\n", duration_since(start_time), -lb_solution.score());
        best_solution = lb_solution;
        return;
    }

    node_t smallest_node = 0;
    while(!remain.test(smallest_node))
        smallest_node++;

    std::vector<bits_t> comps;
    instance.gather_subsets(comps, bits_t().set(smallest_node), remain, smallest_node + 1);
    
    for(bits_t comp: comps) {
        const std::vector<size_t>& choices = instance.by_component(comp);

        for(size_t id: choices) {
            const Comparent& scoreable = instance.get(id);

            Reach new_reach(reach);
            scoreable.update_reach(new_reach);

            if(new_reach.has_cycle())
                continue;
            
            Solution updated_solution(solution);
            updated_solution.add(scoreable);

            new_reach.simplify_past(remain, config.has(SIMPLIFY));
            solve(new_reach, remain &~ comp, updated_solution);
        }
    }
}

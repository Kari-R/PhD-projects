#include <Search.hpp>

#define N instance.size()

Search::Search(const Instance& instance):
    instance(instance), state(instance),
    ub_solution(instance), ub(instance.starting_ub()), bounds(instance) {}

bool Search::update_ub(size_t depth) {
    if(depth > 0)
        info.visited_bottom++;
    
    Weight weight = bounds.calculate_weight(state);
    bool improved = false;
    if(ub > weight) {
        if(depth > 0) {
            header("New UB Found (%)\n",
                format_milliseconds(get_time() - info.start_time));
            state.output();
            print("Weight: %\n", weight);
	    //bounds.output_constraint_status();
        }
        ub = weight;
        ub_solution = state;
        improved = true;
    }
    return improved;
}

#define PERCENTAGE(a, b) \
    (!(a)? 0: double(a) / double(b) * 100)

void Search::print_stats() const {
    size_t time = get_time() - info.start_time;
    
    if(ub == instance.starting_ub()) {
        print("\n** Could not improve the given initial UB of %. **\n\n",
            instance.starting_ub());
    }
    
    header("Search Statistics\n");
    print("Time spent: %\n", format_milliseconds(time));
    print("  To init search: % (% %)\n",
        format_milliseconds(info.time_to_init),
        PERCENTAGE(info.time_to_init, time));
    print("  To evaluate constraints: % (% %)\n",
        format_milliseconds(bounds.constraint_evaluation_time()),
        PERCENTAGE(bounds.constraint_evaluation_time(), time));
    if(!instance[DisableCPLEX])
        print("  In linear programming: % (% %)\n",
            format_milliseconds(bounds.cplex_time()),
            PERCENTAGE(bounds.cplex_time(), time));
    
    print("Visited branches: % (% branches/s)\n"
            "  At bottom: % (% %)\n",
        info.visited_branches,
        size_t(info.visited_branches/(double(time) / 1000)),
        info.visited_bottom,
        PERCENTAGE(info.visited_bottom, info.visited_branches));
    
    print("Good LB calculations: % (% %)\n", info.good_lbs,
        (double(info.good_lbs)/info.visited_branches) * 100);
    
    if(instance[Acyclicity])
        print("Backtracks via cyclic solutions: % (% %)\n",
            info.cyclic_solutions,
            PERCENTAGE(info.cyclic_solutions, info.visited_branches));
    
    if(instance[SymmetryBreaking1])
        print("Backtracks via symmetry rule #1: % (% %)\n",
            info.symmetries1,
            PERCENTAGE(info.symmetries1, info.visited_branches));
    
    if(instance[SymmetryBreaking2])
        print("Backtracks via symmetry rule #2: % (% %)\n",
            info.symmetries2,
            PERCENTAGE(info.symmetries2, info.visited_branches));
    
    if(instance[CorePropagation] || instance[RCFixing])
        print("Backtracks via deduced independences: % (RC fixing etc.)\n", info.extra_indeps);
   
    if(!instance[DisableCPLEX])
        print("CPLEX calls: % (% %)\n",
            bounds.cplex_usage(),
            PERCENTAGE(bounds.cplex_usage(), info.visited_branches));
    
    if(ub < instance.starting_ub()) {
        header("OPTIMAL SOLUTION FOUND\n");
        ub_solution.output();
        print("Weight: %\n", ub);
    }

//    bounds.info().evaluate_solution(ub_solution);
}

size_t Search::update(const Edge& decision) {    
    state.decide(decision);
    return bounds.update(state, decision);
}

void Search::reset(const Edge& decision) {
    state.undecide(decision);
    bounds.reset();
}

bool Search::retains_validity(const Edge& decision, size_t depth) {
    
    if(decision.absent())
        return true;
    
    if(bounds.found_independence(decision.x(), decision.y())) {
        verbose("Can't add edge between % and %: Independence was deduced.\n",
            (int)decision.x() + 1, (int)decision.y() + 1);
        info.extra_indeps++;
        return false;
    }
    
    if(!state.could_add_more_edges(decision.x(), decision.y())) {
        verbose("Close branch: Too large degree.\n");
        return false;
    }
    
    if(decision.is_HH() && decision.present() && instance[MaxBidirCount] != NotLimited &&
            state.bidirected_count() >= (size_t)instance[MaxBidirCount])
        return false;
    
    if(decision.is_TH() || decision.is_HT()) {

        if(instance[Acyclicity]) {
            if(!state.retains_acyclicity(decision.cause(), decision.effect())) {
                verbose("Close branch: Introduces a cycle.\n");
                info.cyclic_solutions++;
                return false;
            }
        }

        if(instance[SymmetryBreaking2] && GraphTools(state, MINIMAL).has_inducing_trail(decision.cause(), decision.effect())) {
            verbose("Close branch: Symmetry breaking rule #2\n");
            info.symmetries2++;
            return false;
        }
    }
    return true;
}

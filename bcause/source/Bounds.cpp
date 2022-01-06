#include <Bounds.hpp>
#include <GraphTools.hpp>
#include <TrivialCorer.hpp>
#include <cmath>

#define N instance.size()

Bounds::Bounds(const Instance& instance):
    instance(instance), cplex(instance, instance[ThreadCount]), cores(instance), stats(instance),
        objective(instance, stats,
            instance[DisableCPLEX]? nullptr: &cplex,
            instance[CorePropagation]? &cores: nullptr) {

    IF_CPLEX(
        cplex_lbs.reserve(N * (N - 1) * 3);

        BENCHMARK("Feeding constraints to CPLEX", {
            for(const Constraint& c: instance.independent) cplex.add_item(c);
            for(const Constraint& c: instance.dependent)   cplex.add_item(c);
            cplex.finalize_expression();
        });
        
        TrivialCorer corer(instance);
        BENCHMARK("Gathering unsatisfiable cores for CPLEX", {
            corer.gather();
            corer.feed_to(cplex, cores, 100000);
            corer.gather_special(cplex);
            cplex.finalize_constraints();
        });
        
        cplex_lbs.push_back( cplex.lowerbound() );
        print("Cores: %   Dataset linking rules: %   Initial LB: %\n",
            corer.core_count(), corer.noncore_count(), cplex_lbs.back());
    )
    cplex_calls = 0;
}

double Bounds::pair_score(uchar x, uchar y) const {
    double general = stats.total_weight(x)[0] + stats.total_weight(x)[1] +
        stats.total_weight(y)[0] + stats.total_weight(y)[1];
    
    return general + stats.get(x, y).max_dep;
}

size_t Bounds::update(const Graph& solution, const Edge& decision) {
    IF_CPLEX(
        cplex_lbs.push_back(cplex_lbs.back());
    );
    objective.update(solution, decision);
    return true;
}

void Bounds::reset() {
    IF_CPLEX( cplex_lbs.pop_back(); )
            
    objective.reset();
}

Weight Bounds::calculate_weight(const Graph& graph) {
    return objective.calculate_final_weight(graph);
}

Weight Bounds::lowerbound(Weight ub) {
    Weight naive_lb = objective.lb();

    IF_CPLEX(
        if(objective.has_more_information()) {
            if(naive_lb < ub && ub <= naive_lb + cplex_lbs.back()) {
                cplex_calls++;
                cplex_lbs.back() = cplex.lowerbound();
                
                if(instance[RCFixing])
                    cplex.update_reduced_cost_fixing(ub);
            }
        }
        return std::max(naive_lb, cplex_lbs.back());
    )
    return naive_lb;
}

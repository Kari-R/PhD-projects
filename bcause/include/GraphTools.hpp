#pragma once

#include <Graph.hpp>
#include <CoreSet.hpp>
#include <CPLEX.hpp>

class GraphTools {
public:
    enum Separation {
        D_SEPARATION, SIGMA_SEPARATION
    };
    
    GraphTools(const Graph& graph, bool com_type,
            typename Completion::Fun fun = Completion::default_fun):
        completion(graph, com_type, fun) {}
    
    GraphTools(const Completion&);

    bool has_inducing_trail(uchar, uchar);
    bool has_nontrivial_inducing_trail(uchar, uchar);
    
    bool has_active_trail(const Constraint&);

    bool has_walk_between(uchar, uchar) const; 
    bool has_directed_path(uchar, uchar) const;

private:
    const Completion completion;
};
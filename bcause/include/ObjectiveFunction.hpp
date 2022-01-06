#pragma once

#include <GraphEvaluator.hpp>

class ObjectiveFunction:
    public GraphEvaluator {    
public:
    using List = std::list<const Constraint*>;
    
    ObjectiveFunction(const Instance&, NodeStats&,
        CPLEX* cplex = nullptr, CoreSet* cores = nullptr);
    
    void update(const Graph&, const Edge&);

    void set(List::iterator&, List&, bool);
    bool reset();
    void clear_progress();
    
    Weight calculate_final_weight(const Graph&) const;
    
    inline size_t total_evaluation_time() const { return used_time; }
    
private:
    void update_list(List& list, const Bottleneck&, GraphTools&, bool);
    
    std::vector<List> lists;
    size_t used_time;
};
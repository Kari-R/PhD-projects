#pragma once

#include <Instance.hpp>
#include <Graph.hpp>
#include <BitMatrix.hpp>
#include <ObjectiveFunction.hpp>
#include <CoreSet.hpp>
#include <vector>

class Bounds {
public:
    Bounds(const Instance&);
    
    double pair_score(uchar, uchar) const;

    size_t update(const Graph&, const Edge&);
    
    void reset();
    
    Weight calculate_weight(const Graph&);
    
    Weight lowerbound(Weight);

    inline void clear_progress() {
        objective.clear_progress();
        while(cplex_lbs.size() > 1)
            cplex_lbs.pop_back();
    }
    
    inline const NodeStats& info() const { return stats; }
    
    inline Weight dep_weight(uchar x, uchar y) const {
        return stats.total_weight(x, y)[0];
    }
    
    inline Weight indep_weight(uchar x, uchar y) const {
        return stats.total_weight(x, y)[1];
    }
    
    inline size_t cplex_usage() const { return cplex_calls; }
    
    inline bool found_independence(uchar x, uchar y) const {
        return cores.induced_independence(x, y) ||
            cplex.induced_independence(x, y);
    }
    
    inline size_t cplex_time() const { return cplex.time_spent_solving(); };
    inline size_t constraint_evaluation_time() const { return objective.total_evaluation_time(); };

    CPLEX& get_cplex() { return cplex; }
    
private:
    const Instance& instance;
    CPLEX cplex;
    CoreSet cores;
    NodeStats stats;
    std::vector<Weight> cplex_lbs;
    ObjectiveFunction objective;
    size_t cplex_calls;
};

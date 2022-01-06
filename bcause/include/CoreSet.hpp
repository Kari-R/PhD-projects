#pragma once

#include <Instance.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>
#include <list>

class CoreSet {
public:
    typedef std::function<void(const Constraint&)> Fun;
    
    struct Core {
        std::list<Constraint> items;
        uchar undefs_left, unsats;
        
        Core(const std::list<Constraint>& constraints):
            items(constraints),
            undefs_left(constraints.size()),
            unsats(0) {}
    };
    
    CoreSet(const Instance&);
    
    void add(const std::list<Constraint>&);
    
    void mark_satisfied(const Constraint&);
    void mark_unsatisfied(const Constraint&);
    
    void reset(const Constraint&);
    
    const Constraint& identify_the_remaining(const Core&) const;
    
    inline void open_scope() {
        propagation_levels.push_back(std::vector<const Constraint*>());
    }
    
    void close_scope();
    
    inline bool induced_independence(uchar x, uchar y) const {
        return y < x? induced_independence(y, x):
            unsat_deps[x][y] > 0;
    }
    
    inline size_t identified_count() const { return status.size(); }

private:
    const Instance& instance;
    
    std::list<Core> cores;
    std::unordered_map<Constraint::Id, std::vector<Core*> > lookup;
    
    std::unordered_map<Constraint::Id, bool> status;
    
    std::unordered_set<Constraint::Id> propagated_unsats;
    std::vector<std::vector<size_t> > unsat_deps;
    
    std::vector<std::vector<const Constraint*> > propagation_levels;
};
#pragma once

#include <Instance.hpp>
#include <Graph.hpp>
#include <Bounds.hpp>

#define verbose(...) \
    { if(instance[Verbose]) {\
        ADD_SPACING\
        print("| ");\
        print(__VA_ARGS__);\
    }}

class Search {
public:    
    struct Info {
        size_t start_time, cyclic_solutions,
            good_lbs, visited_branches, visited_bottom,
            symmetries1, symmetries2, time_to_init,
            extra_indeps;
        
        Info(): start_time(get_time()),
            cyclic_solutions(0), good_lbs(0),
            visited_branches(0), visited_bottom(0), symmetries1(0),
            symmetries2(0), time_to_init(0), extra_indeps(0) {}
    };
    
    Search(const Instance&);

    size_t update(const Edge&);
    void reset(const Edge&);

    bool update_ub(size_t depth = 0);
    void print_stats() const;
    bool retains_validity(const Edge&, size_t);

protected:
    const Instance& instance;
    Info info;
    Graph state, ub_solution;
    Weight ub;
    Bounds bounds;
};
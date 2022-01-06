#pragma once

#include <Instance.hpp>
#include <CoreSet.hpp>
#include <CPLEX.hpp>

class TrivialCorer {
public:
    TrivialCorer(const Instance&);
    
    void gather();
    void gather(Bits);
    
    void gather_special(CPLEX&, const std::vector<Constraint::Id>&, Bits);
    void gather_special(CPLEX&);
    
    void feed_to(CPLEX& cplex, CoreSet& coreset, size_t max_cores = INF);
    
    inline size_t core_count() const { return fed_to_cplex; }
    inline size_t noncore_count() const { return noncores_fed; }
    
protected:
    void consider1(Bits, Bits, uchar, uchar, uchar);
    void consider2(Bits, Bits, uchar, uchar, uchar);
    void consider3(Bits, Bits, uchar, uchar, uchar);
    void consider4(Bits, Bits, uchar, uchar, uchar, uchar);
    void consider5(Bits, Bits, uchar, uchar, uchar, uchar);
    void consider6(Bits, Bits, uchar, uchar, uchar, uchar);
    void consider7(Bits, Bits, uchar, uchar, uchar, uchar);
    
    inline void add_core(const std::list<Constraint>& core) {
        found++;
        cores_by_size[core.size()].push_back(core);
    }

private:
    const Instance& instance;
    size_t found, fed_to_cplex, noncores_fed;
    std::list< std::list<Constraint> > cores_by_size[6];
};
#pragma once

#include <Search.hpp>

class BranchAndBound:
    public Search {    
public:
    BranchAndBound(const Instance&);
    
    void solve();
    
    bool add_edge_first(uchar x, uchar y) const;
    
    inline double weight_ratio(uchar x, uchar y) const {
        return ((double)initial_dep[x][y] + 1) /
            ((double)initial_indep[x][y] + 1);
    }
    
protected:
    void apply_edge_fixings();
    
    void select_next_edge(size_t, uchar);
    bool choose_edge(const Edge*, const Edge&, uchar);
    void gather_edge(std::vector<Edge>&, const Edge&);
    void branch(const Edge&, size_t, uchar);
    
    void find_naive_UB();
    void gather_initial_information();
    
private:
    std::vector<std::vector<Weight> > initial_indep, initial_dep;
    Weight median_dep, avg_dep, avg_ratio;
    Bits is_pointed;
};
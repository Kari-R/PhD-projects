#pragma once

#include <ParentSet.hpp>
#include <BayesSolver.hpp>
#include <Moralizer.hpp>
#include <vector>

class MarkovBounder {
public:
    typedef std::vector<std::bitset<32> > Cliques;
    
    MarkovBounder(const Solution&, Instance&, uint);

    Score tight_upperbound(size_t, Score);
    
    inline Score fast_upperbound() {
        return state.score + bayes_solver.solve(state.assign_mask);
    }
    
    Network lowerbound_solution(size_t);
    Network upperbound_solution();
    
    size_t best_final_parentset(uint) const;
    void construct_naive_cliques(Cliques&);

    bool has_path_to(uint, uint, std::bitset<32>&, std::bitset<32>&);
    void find_bad_cliques(uint, Cliques&);
    
    void construct_tight_cliques(Cliques&, const Cliques&, const Cliques&);
    void decompose_clique(Cliques&, std::bitset<32>, const Cliques&,
        size_t, std::unordered_set<std::bitset<32> >*);

protected:
    size_t find_best_relaxedly_moral_parentset(const Cliques*, uint, const std::bitset<32>&);
    size_t find_random_valid_parentset(const ArcMatrix&, uint, const std::bitset<32>&);
    
    Score upperbound(const Cliques*, std::bitset<32>, size_t, Score, Score);
    
private:
    const Solution& state;
    Instance& instance;
    BayesSolver bayes_solver;
    uint N;
    std::unordered_map<std::bitset<32>, Score> seen;
};
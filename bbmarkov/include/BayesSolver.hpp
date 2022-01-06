#pragma once

#include <Instance.hpp>
#include <ParentSet.hpp>
#include <ScoreTree.hpp>
#include <Moralizer.hpp>

class BayesSolver {
public:    
    BayesSolver(Instance&, uint);

    void construct_solution(ArcMatrix&, std::bitset<32>) const;

    Score solve(std::bitset<32>);

protected:
    uint find_best_variable(std::bitset<32>) const;
    size_t find_best_parentset(uint, const std::bitset<32>&) const;
    
    void assign_next_variable(ArcMatrix&, std::bitset<32>&) const;

private:
    Instance& instance;
    uint N;
    std::vector<Score> scores;
};
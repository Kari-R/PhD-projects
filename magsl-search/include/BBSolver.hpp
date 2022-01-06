#pragma once

#include <DPSolver.hpp>
#include <Solution.hpp>

class BBSolver {
public:
    BBSolver(const Config&, const Scores&);

    void solve(size_t, const Solution* start_solution = nullptr);
    void solve(const Reach&, bits_t, const Solution&);
    
private:
    const Config& config;
    const Scores& instance;
    DPSolver dp;
    size_t iterations, nonmaximals, start_time;
    Solution best_solution;
};

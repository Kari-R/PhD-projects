#pragma once

#include <Reach.hpp>
#include <Scores.hpp>
#include <Solution.hpp>

class DPSolver {
public:
    DPSolver(const Config& config, const Scores& scores):
        config(config), instance(scores),
        iterations(0), cache_hits(0),
        result(instance.size()), find_BN(false) {}

    const Solution& solve(const Solution&, const Reach&, bits_t);
    
    score_t solve(const Reach&, bits_t, bool printing = false);

    const Solution& learn_BN() {
    	result = Solution(instance.size(), 0);
	Reach reach(instance.size());
	find_BN = true;
    	solve(reach, filled_bitset<bits_t>(instance.size()), true);
	find_BN = false;
    	return result;
    }
    
private:
    const Config& config;
    const Scores& instance;
    std::unordered_map<Reach::Key, score_t> cache;
    size_t iterations, cache_hits;
    Solution result;
    bool find_BN;
};

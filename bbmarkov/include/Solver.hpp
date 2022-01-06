#pragma once

#include <Solution.hpp>
#include <DomainBuilder.hpp>
#include <MarkovBounder.hpp>
#include <Network.hpp>
#include <unordered_set>
#include <unordered_map>
#include <fstream>

struct Statistics {
    std::vector<size_t> layer_visits;
    size_t start_time;
    size_t skeleton_hits, option_hits;
    size_t tight_UBs, successful_tight_UBs;
    Score LB, UB;
    
    inline size_t time() const {
        return get_time() - start_time;
    }
};

static const std::string& flag_symbols = "vVsoftu";

enum Flag {
    MinVerbosity,
    MaxVerbosity,
    MemoizeSkeletons,
    MemoizeOptions,
    FixedSourceVertex,
    TightUpperbounds,
    SortByUpperbounds
};

class Solver {
public:
    Solver(const std::bitset<32>&, Instance&, uint);
    
    void solve();
    const Solution& solution() const;

protected:
    void output_lowerbound(const std::string& title);
    void output_statistics();
    void track_skeletons();
    
    Score generate_lowerbound();
    Score choose_last_parentset();
    Score solve(uint, size_t edges = 0);

private:
    uint N;
    std::bitset<32> flags;
    Statistics stats;
    Instance& instance;
    
    Solution state;
    std::unordered_map<
        ArcMatrix,
        Solution,
        NetworkHash,
        NetworkCompare> skeleton_memo;
    std::unordered_map<
        OptionList,
        Score,
        OptionsHash,
        OptionsCompare> option_memo;
    MarkovBounder bounds;
    
public:
    Solution lb_solution;
};
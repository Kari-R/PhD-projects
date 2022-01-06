#pragma once

#include <ParentSet.hpp>
#include <Instance.hpp>
#include <Solution.hpp>
#include <set>
#include <unordered_map>
#include <functional>
#include <debug.hpp>

typedef std::function<Score(uint, size_t)> ValueCalculator;

struct OptionsSort {
    ValueCalculator * calculate_ub;
    const Domains& domains;
    
    bool operator()(size_t, size_t) const;
};

typedef std::set<size_t, OptionsSort> OptionList;

struct OptionsHash {
    inline size_t operator()(const OptionList& list) const {
        size_t hash = 0;
        for(size_t value: list)
            hash ^= (value * 1103515245 + 12345) % ((size_t(1) << 31) - 1);
        return hash;
    }
};

struct OptionsCompare {
    inline bool operator()(const OptionList& a, const OptionList& b) const {
        if(a.size() != b.size())
            return false;
        auto it1 = a.begin();
        auto it2 = b.begin();
        while(it1 != a.end()) {
            if(*it1 != *it2)
                return false;
            it1++;
            it2++;
        }
        return true;
    }
};

class DomainBuilder {
public:
    static uint get_depth(const Solution&, const ParentSet&);
    
    DomainBuilder(Instance&, Solution&, ValueCalculator *, uint);
    
    bool in_same_subgraph(std::bitset<32>&, uint, uint);
    
    Score build(uint, const std::bitset<32>&);
    Score build_supersets(uint, std::bitset<32>);

    inline const OptionList& list() const {
        return option_list;
    }
    
    bool worsens_lexicographicality(uint, const ParentSet&);
    void construct_for(uint);

private:
    uint N, latest_var;
    Instance& instance;
    Solution& state;
    
    std::unordered_map<std::bitset<32>, Score> superset_scores;
    OptionList option_list;
    ValueCalculator * calculate_ub;
};
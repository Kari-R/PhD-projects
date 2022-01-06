#pragma once

#include <ParentSet.hpp>

class ScoreTree {
public:
    ScoreTree(uint);

    void construct(const Domain&, const DomainLookup&);
    size_t construct(std::bitset<32>);
    
    inline size_t get(const std::bitset<32>& subset) const {
        return sequences[subset.to_ulong()];
    }

    const ParentSet& best_within(const std::bitset<32>& subset) const {
        return (* domain)[sequences[subset.to_ulong()]];
    }

private:
    uint N;
    const Domain* domain;
    const DomainLookup* domain_lookup;
    std::vector<size_t> sequences;
};

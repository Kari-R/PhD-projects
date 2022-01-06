#pragma once

#include <ParentSet.hpp>
#include <Network.hpp>
#include <vector>
#include <bitset>

struct Vertex {
    uint depth, parentset;
};

struct Solution {
    Score score;
    std::vector<Vertex> vertexes;
    ArcMatrix skeleton;
    std::bitset<32> assign_mask;
    
    Solution(Score s, uint size, uint max_var):
        score(s),
        vertexes(size, Vertex {max_var, 0}),
        skeleton(size, std::bitset<32>(0)) {}

    inline bool contains(uint var) const {
        return assign_mask.test(var);
    }
    
    inline const Vertex& operator[](uint i) const {
        return vertexes[i];
    }
    
    inline Vertex& operator[](uint i) {
        return vertexes[i];
    }
    
    void print(const Domains&) const;
    
    static Solution construct(const Domains&,
        const std::vector<DomainLookup>&, const Network&);
    
    void set_parents(uint, const ParentSet&);
    void reset_parents(uint);
    
    std::string fingerprint() const;
};

uint find_root(const Solution&, const Domains&, uint, std::vector<uint>&);
void find_order(const Solution&, const Domains&, std::vector<uint>&);
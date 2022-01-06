#pragma once

#include <bitset>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <bitset>
#include <debug.hpp>

typedef double Score;

#define INF 9999999

struct ParentSet {
    std::bitset<32> content;
    std::vector<uint> list;
    Score score;

    ParentSet(Score s): score(s) {}
    
    void add(uint vertex) {
        content.set(vertex, true);
        list.push_back(vertex);
    }
    
    inline size_t count() const {
        return list.size();
    }
    
    inline uint operator[](size_t i) const {
        return list[i];
    }
};

inline std::bitset<32> full_set(uint N) {
    return std::bitset<32>((size_t(1) << N) - 1);
}

typedef std::vector<ParentSet> Domain;
typedef std::vector<Domain> Domains;
typedef std::unordered_map<std::bitset<32>, size_t> DomainLookup;

std::ostream& operator<<(std::ostream&, const ParentSet&);

void initialize_lookup_table(std::vector<DomainLookup>&, const Domains&);

void read_cussen_scores(std::istream&, Domains&);
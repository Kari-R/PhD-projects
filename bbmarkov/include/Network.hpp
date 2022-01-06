#pragma once

#include <ParentSet.hpp>
#include <unordered_set>

typedef std::vector<std::bitset<32> > ArcMatrix;

struct Network {
    ArcMatrix matrix;
    Score score;

    Network(ArcMatrix&& m, Score s): matrix(std::move(m)), score(s) {}
    Network(const ArcMatrix& m, Score s): matrix(m), score(s) {}
    
    Network(Network&& n): matrix(std::move(n.matrix)), score(n.score) {}
    Network(const Network& n): matrix(n.matrix), score(n.score) {}
    
    Network& operator=(Network&& n) { matrix.swap(n.matrix); score = n.score; }
    Network& operator=(const Network& n) { matrix = n.matrix; score = n.score; }
    
    bool operator<(const Network&) const;
    size_t count_immoralities() const;
    
    bool has_path_between(uint, uint, std::bitset<32>&);
    bool is_within_cycle(uint);
};

struct NetworkHash {
    size_t operator()(const ArcMatrix&) const;
};

struct NetworkCompare {
    bool operator()(const ArcMatrix&, const ArcMatrix&) const;
};

typedef std::unordered_set<ArcMatrix, NetworkHash, NetworkCompare> NetworkSet;
#pragma once

#include <Solution.hpp>
#include <Instance.hpp>
#include <ParentSet.hpp>
#include <Network.hpp>
#include <unordered_set>
#include <unordered_map>
#include <queue>

class Moralizer {
public:
    enum Direction {
        AddEdges, RemoveEdges
    };
    
    typedef std::priority_queue<Network> Queue;
    
    Moralizer(Instance&, uint);
    
    size_t count_immoralities(const ArcMatrix&);
    
    Network moralize(const ArcMatrix&, Direction, size_t);
    Network moralize(const ArcMatrix&, size_t);
    
protected:
    std::bitset<32> parentset_of(const ArcMatrix&, uint);
    
    void fix_global_immoralities(const Network&, Network&,
        Direction, size_t, size_t);
    
    void clear_candidates(Queue&, size_t);
    
    void consider_edge_addition(Queue&, Network, uint, uint);
    void consider_edge_removal(Queue&, Network, uint, uint);
    
    size_t find_immoralities(Queue&, const Network&, Direction);

private:
    Instance& instance;
    NetworkSet visited;
    uint max_var;
};

#pragma once

#include <Reach.hpp>
#include <Comparent.hpp>

class Solution {
public:
    Solution(node_t);
    Solution(node_t, score_t);
    
    void add_parent(node_t, node_t);
    void add_bidirected(node_t, node_t);
    
    inline bool has_edge(node_t x, node_t y) const {
        return edges[x].test(y) && !edges[y].test(x);
    }
    
    inline bool has_bidirected_edge(node_t x, node_t y) const {
        return edges[x].test(y) && edges[y].test(x);
    }
    
    void add(const Comparent&);
    
    bool is_connected(node_t, node_t, bits_t visited = 0) const;
    
    bool has_inducing_path(node_t, node_t, node_t, bits_t, size_t) const;
    bool is_maximal(bits_t remain = 0) const;
    bool has_cycle() const { return reach.has_cycle(); }
    
    inline size_t size() const { return edges.size(); }
    inline score_t score() const { return score_; }

    void build_c_component(node_t, bits_t&) const;
    
    void output(const std::string& name = "Solution") const;
    
private:
    Reach reach;
    std::vector<bits_t> edges;
    score_t score_;
};

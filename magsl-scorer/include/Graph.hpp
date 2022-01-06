#pragma once

#include <Reach.hpp>
#include <ICF.hpp>

class Graph {
public:
    Graph(node_t);
    
    static Graph from_edge_matrix(std::istream&);
    static Graph from_ccd_output(std::istream&);
    static Graph from_own_output(std::istream&);
    
    void add_parent(node_t, node_t);
    void add_bidirected(node_t, node_t);
    
    inline bool has_edge(node_t x, node_t y) const {
        return edges[x].test(y) && !edges[y].test(x);
    }
    
    inline bool has_bidirected_edge(node_t x, node_t y) const {
        return edges[x].test(y) && edges[y].test(x);
    }

    bool is_connected(node_t, node_t, bits_t visited = 0) const;
    bool is_strongly_connected() const;
    
    bool has_inducing_path(node_t, node_t, node_t, bits_t, size_t) const;
    bool is_maximal(bits_t remain = 0) const;
    bool has_cycle() const { return reach.has_cycle(); }
    
    inline size_t size() const { return edges.size(); }
    bool is_complete() const;
    
    void output() const;
    
    std::vector<Scoreable> as_scoreables() const;
    double score(const ICF&, size_t, size_t) const;
    
    bool has_bad_indpath(node_t, node_t, bits_t) const;
    bool has_bad_indpath() const;

private:
    Reach reach;
    std::vector<bits_t> edges;
};


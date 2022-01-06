#pragma once

#include <util.hpp>
#include <Instance.hpp>
#include <vector>
#include <unordered_set>
#include <set>
#include <list>
#include <cstdlib>

class NeighborSelector;

class Graph {
public:
    enum Status {
        UNKNOWN,
        TOO_WIDE,
        NOT_CHORDAL,
        BDEU_OVERFLOW,
        FEASIBLE,
    };
    
    static size_t& scoring_time() { static size_t t = 0; return t; }
    static size_t& ordering_time() { static size_t t = 0; return t; }
    
    friend class NeighborSelector;

    void add(const Bitset& clique) {
        for(var_t v1: clique.contents())
        for(var_t v2: clique.contents())
            if(v1 != v2)
                add_edge(v1, v2);
    }
    
    void add_edge(var_t x, var_t y) {
        status = UNKNOWN;
        cached_score = INF;
        ensure(x < size() && y < size() && x != y);
//        ensure(!adjacency[x].test(y) && !adjacency[y].test(x));
        adjacency[x].set(y);
        adjacency[y].set(x);
    }
    
    void remove_edge(var_t x, var_t y) {
        status = UNKNOWN;
        cached_score = INF;
        ensure(x < size() && y < size() && x != y);
//        ensure(adjacency[x].test(y) && adjacency[y].test(x));
        adjacency[x].reset(y);
        adjacency[y].reset(x);
    }
    
    void init_tree();
    
    inline bool contains_edge(var_t x, var_t y) {
        ensure(x < size() && y < size() && x != y);
        return adjacency[x].test(y);
    }
    
    Graph(const Instance& inst):
        instance(&inst),
        adjacency(inst.max_size(), Bitset()),
        cached_score(INF), status(UNKNOWN) {}
    
    void init_rand();

    score_t score() const;
    
    bool is_feasible() const;
    
    score_t get_score(var_t, const Bitset&) const;
    
    void evaluate(const std::vector<var_t>&) const;
    
    var_t pick_next_variable(const std::list<Bitset>&, const Bitset&) const;
    
    void partition_sets(std::list<Bitset>&, var_t, const Bitset&) const;
    
    var_t find_next_lex_variable(std::list<Bitset>&, Bitset&) const;
    
    void find_lex_order(std::vector<var_t>&) const;
    
    inline size_t size() const { return instance->max_size(); }
    
    const Bitset& connections(var_t var) const{
        return adjacency[var];
    }
    
    void extract_cliques(std::vector<Bitset>&) const;
    
    friend std::ostream& operator<<(std::ostream&, const Graph&);
    
private:
    const Instance* instance;
    std::vector<Bitset> adjacency;
    mutable score_t cached_score;
    mutable Status status;
};

std::ostream& operator<<(std::ostream&, const Graph&);

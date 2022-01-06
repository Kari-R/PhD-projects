#pragma once

#include <util.hpp>
#include <Reach.hpp>

using score_t = double;

#define MAX_SCORE score_t(1000*1000*1000)

class Comparent {
public:
    Comparent(bits_t, score_t);
    
    inline const Set& component() const { return comp; }

    inline const Set& parents(node_t node) const {
        ensure(comp.has(node));
        ensure(node < node_to_index.size());
        return psets[ node_to_index[node] ];
    }
    
    inline bits_t get_absent_edges(node_t node) const {
        ensure(comp.has(node));
        return absent_edges[node_to_index[node]];
    }
    
    void add_parents(bits_t);
    
    inline score_t score() const { return score_; }
    
    void add_missing_edge(bits_t);
    bool has_edge(node_t, node_t) const;
    void update_reach(Reach&) const;
    
    inline node_t order(node_t node) const {
        ensure(component().has(node));
        return node_to_index[node];
    }

    friend std::ostream& operator<<(std::ostream&, const Comparent&);
    
    void output_to(std::ostream&) const;
    
    struct Key {
        Key(const Comparent&);
        Key(bits_t, const Comparent&);
        
        inline size_t hash() const {
            size_t h = 0;
            for(auto e: sets)
                h ^= std::hash<int>{}(e) + 0x9e3779b9 + (h << 6) + (h >> 2); 
            return h;
        }
        
        inline size_t component() const { return sets[1]; }
        
        inline bits_t parents(size_t i) const {
            return sets[i + 2];
        }
        
        Key& add_missing_edge(const Comparent&, node_t, node_t);
        Key& remove_missing_edge(const Comparent&, node_t, node_t);
        
        Key& set_parents(node_t, bits_t);
        
        bool operator==(const Key&) const;
        void debug();
        
        std::vector<size_t> sets;
    };
    
private:
    score_t score_;
    Set comp;
    std::vector<node_t> node_to_index;
    std::vector<Set> psets;
    bits_t missing_edge;
    std::vector<bits_t> absent_edges;
};

namespace std {
    template<>
    struct hash<Comparent::Key> {
        inline size_t operator()(const Comparent::Key& key) const {
            return key.hash();
        }
    };
}

std::ostream& operator<<(std::ostream&, const Comparent&);

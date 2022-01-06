#pragma once

#include <ICF.hpp>
#include <unordered_map>

struct Pruneable {
    std::vector<size_t> data;

    Pruneable(const Scoreable& s): data(s.component_size() * 2 + 1, 0) {
        for(size_t x = 0; x < s.component_size(); x++) {
            for(node_t y: s.parents[x])
                add_parent(x, s.nodes[y]);
            for(size_t y: s.spouses[x])
                add_spouse(x, s.nodes[y]);
            data.back() |= (size_t(1) << s.nodes[x]);
        }
    }
    
    Pruneable(const Pruneable& p, bits_t selection) {
        ensure(selection.count() != 0 && selection.count() < p.node_count());
        bits_t vars;
        for(size_t x = 0, i = 0; x < bits_t().size(); x++)
            if(p.component().test(x)) {
                if(selection.test(i))
                    vars.set(x);
                i++;
            }
        for(size_t x = 0; x < p.node_count(); x++)
            if(selection.test(x))
                data.push_back(p.data[x] & vars.to_ulong());
        for(size_t x = 0; x < p.node_count(); x++)
            if(selection.test(x))
                data.push_back(p.data[p.node_count() + x] & vars.to_ulong());
        data.push_back(vars.to_ulong());
    }
    
    inline bits_t component() const { return data.back(); }
    
    bool operator==(const Pruneable& p) const {
        if(data.size() != p.data.size())
            return false;
        for(size_t i = 0; i < data.size(); i++)
            if(data[i] != p.data[i])
                return false;
        return true;
    }

    #define get_spouses(x) data[node_count() + (x)]

    bool has_spouse(node_t i, node_t j) const {
        return get_spouses(i) & (size_t(1) << j);
    }

    void remove_spouse(node_t i, node_t j) {
        get_spouses(i) &= ~(size_t(1) << j);
    }

    void add_spouse(node_t i, node_t j) {
        get_spouses(i) |= (size_t(1) << j);
    }

    inline bool has_parent(node_t i, node_t j) const {
        return data[i] & (size_t(1) << j);
    }

    inline void remove_parent(node_t i, node_t j) {
        data[i] &= ~(size_t(1) << j);
    }

    inline void add_parent(node_t i, node_t j) {
        data[i] |= (size_t(1) << j);
    }
    
    inline node_t node_count() const { return (data.size() - 1)/2; }
};

namespace std {
    template<>
    struct hash<Pruneable> {
        size_t operator()(const Pruneable& key) const {
            size_t h = 0;
            for(auto e: key.data)
                h ^= std::hash<size_t>{}(e) + 0x9e3779b9 + (h << 6) + (h >> 2); 
            return h;
        }
    };
}

using ScoredItem = std::pair<Scoreable, double>;

class Pruner {
public:
    struct Status {
        double max_score;
        bool is_processed;
    };
    
    Pruner(size_t prune_level):
        prune_level(prune_level), prune_count(0), prune_time(0), total_multiple_optimas(0), most_optimas(0) {}
    
    inline void add(const Scoreable& s, double score) {
        scores.push_back({s, score});
    }
    
    void mark_optima_count(const Scoreable& s, size_t count) {
        if(count > 1) {
            optima_count.insert({s, count});
            total_multiple_optimas++;
        }
        if(most_optimas < count)
            most_optimas = count;
    }

    void prune();
    
    inline const std::vector<ScoredItem>& get() const { return scores; }
    
    void clear() {
        scores.clear();
    }
    
    inline size_t pruned_score_count() const { return prune_count; }
    inline size_t total_prune_time() const { return prune_time; }
    
    std::pair<size_t, size_t> optima_rate_after_pruning() const;
    std::pair<size_t, size_t> optima_rate_before_pruning() const {
        return {total_multiple_optimas, most_optimas};
    }
    
private:
    double get_max_score(Pruneable&);
    double get_max_partition_score(Pruneable&);
    double get_max_sub_score(Pruneable&);
    
    std::unordered_map<Pruneable, Status> cache;
    std::vector<ScoredItem> scores;
    size_t prune_level, prune_count, prune_time, total_multiple_optimas, most_optimas;
    std::unordered_map<Pruneable, size_t> optima_count;
};

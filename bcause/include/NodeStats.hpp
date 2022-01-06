#pragma once

#include <Instance.hpp>
#include <Graph.hpp>
#include <vector>
#include <unordered_map>

#define RUNNING_STATS true

enum Status {
    Returned = -1,
    Unchanged = 0,
    Removed = 1
};

class NodeStats {
public:
    struct Extremas {
        Weight max_dep, min_dep,
            max_indep, min_indep;
        size_t dep_count, indep_count;

        Extremas():
            max_dep(0), min_dep(INF),
            max_indep(0), min_indep(INF),
            dep_count(0), indep_count(0) {}
    };
    
    void evaluate_solution(const Graph&) const;

    NodeStats(const Instance&);
    
    inline const Extremas& get(uchar v) const {
        ensure(!scopes.empty());
        return scopes.back().nodes[v];
    }
    
    inline const Extremas& get(uchar x, uchar y) const {
        ensure(!scopes.empty());
        auto it = scopes.back().pairs.find(x + y * instance.size());
        //ensure(it != scopes.back().pairs.end());
        if(it == scopes.back().pairs.end())
            return nonexistent_extremas;
        return it->second;
    }
    
    inline const std::vector<Weight>& total_weight(uchar v) const {
        return nodes[v];
    }
    
    inline const std::vector<Weight>& total_weight(uchar x, uchar y) const {
        return pairs[x][y];
    }

    void update_extremas(const Constraint&);
    
    template<Status status>
    inline void mark_all(bool indep, uchar x, uchar y) {
        nodes[x][indep] -= pairs[x][y][indep] * status;
        nodes[y][indep] -= pairs[x][y][indep] * status;
    }
    
    template<Status status>
    inline void mark(const Constraint& c) {
        if(status == Unchanged) {
            #if RUNNING_STATS
                update_extremas(c);
            #endif
        } 
        else {
            pairs[c.v1][c.v2][c.independence] -= c.weight * status;
            nodes[c.v1][c.independence] -= c.weight * status;
            nodes[c.v2][c.independence] -= c.weight * status;
        }
    }
    
    void open_scope() {
        #if RUNNING_STATS
            scopes.push_back(Scope(instance));
        #endif
    }

    void close_scope() {
        #if RUNNING_STATS
            ensure(!scopes.empty());
            scopes.pop_back();
        #endif
    }
    
    inline Bits potential_dependences(uchar var) const {
        return potential_deps[var];
    }
    
protected:
    void initialize(const std::vector<Constraint>&);
    
    struct Scope {
        std::unordered_map<size_t, Extremas> pairs;
        std::vector<Extremas> nodes;
        
        Scope(const Instance& instance):
            nodes(instance.size(), Extremas()) {}
    };
    
private:
    const Instance& instance;
    std::vector<std::vector<Weight> > nodes;
    std::vector<std::vector<std::vector<Weight> > > pairs;
    std::vector<Bits> potential_deps;
    std::vector<Scope> scopes;
    Extremas nonexistent_extremas;
};
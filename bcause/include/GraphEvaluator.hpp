#pragma once

#include <Instance.hpp>
#include <Graph.hpp>
#include <GraphTools.hpp>
#include <CPLEX.hpp>
#include <NodeStats.hpp>
#include <CoreSet.hpp>
#include <vector>
#include <unordered_set>

class GraphEvaluator {
public:
    static constexpr char Undefined = -1;
    
    struct Bottleneck {
        Bits colliders, noncolliders;
        
        Bottleneck() {}
        Bottleneck(uchar N):
            colliders(full_set(N)),
            noncolliders(full_set(N)) {}
        
        Bottleneck& operator|=(const Bottleneck& l) {
            colliders |= l.colliders;
            noncolliders |= l.noncolliders;
            return *this;
        }
        
        Bottleneck& operator&=(const Bottleneck& l) {
            colliders &= l.colliders;
            noncolliders &= l.noncolliders;
            return *this;
        }
        
        inline bool is_reducable_by(const Bottleneck& bn) const {
            return (colliders & bn.colliders) != colliders ||
                (noncolliders & bn.noncolliders) != noncolliders;
        }
    };

    GraphEvaluator(const Instance&, NodeStats&, CPLEX* cplex = nullptr, CoreSet* cores = nullptr);
    
    void set(const Constraint*, bool);
        
    inline Weight lb() const { return min_weight.back(); }
    
    uchar status(const Constraint& c) const {
        auto it = statuses.find(c.ident());
        return it == statuses.end()? (uchar)CPLEX::Undefined:
            it->second;
    }
    
    Weight naive_lb(const Graph&) const;
    
    inline void open_scope(Weight weight) {
        min_weight.push_back(weight);
        removed.push_back(std::vector<const Constraint*>());
        if(cores != nullptr)
            cores->open_scope();
        if(cplex != nullptr)
            cplex->open_scope();
    }
    
    inline void close_scope() {
        removed.pop_back();
        min_weight.pop_back();
        if(cores != nullptr)
            cores->close_scope();
        if(cplex != nullptr)
            cplex->close_scope();
    }
    
    inline bool has_more_information() const {
        return min_weight.size() <= 1 ||
            min_weight.back() != min_weight[min_weight.size() - 2];
    }
    
    inline size_t depth() const { return min_weight.size(); }

protected:    
    Bottleneck naive_bottleneck(const Completion&, uchar, uchar, Bits, const Edge&) const;

    const Instance& instance;
    NodeStats& stats;
    CPLEX* cplex;
    CoreSet* cores;
    
    std::vector< std::vector<const Constraint*> > removed;
    std::unordered_map<Constraint::Id, bool> statuses;
    std::vector<Weight> min_weight;
};
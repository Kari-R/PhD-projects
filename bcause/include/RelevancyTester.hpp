#pragma once

#include <GraphEvaluator.hpp>

class RelevancyTester {
public:
    struct BottleNode {
        Bits bottlenecks, relevant_neighbours;
        bool leads_to_goal;
        
        BottleNode(uchar N):
            bottlenecks(full_set(N)),
            relevant_neighbours(),
            leads_to_goal(false) {}

        bool is_reduced_by(Bits bnecks) const {
            return (bottlenecks & bnecks) != bottlenecks;
        }
        
        Bits reduce_by(Bits bnecks) {
            return bottlenecks = (bottlenecks & bnecks);
        }
    };
    
    RelevancyTester(const Completion& graph, const Edge& decision):
        graph(graph), decision(decision) {}

    bool relevant_for(uchar, uchar);
    
    inline GraphEvaluator::Bottleneck bottleneck() const {
        return unavoidability;
    }
    
protected:
    bool relevant_v_structure(uchar);
    
    bool inspect_trails();
    bool effect_could_be_collider(uchar, Bits&);
    
    inline bool relevant_neighbour(uchar node, uchar neighbour) const {
        return (bottles[0][node].relevant_neighbours |
            bottles[1][node].relevant_neighbours).test(neighbour);
    }
    
    inline bool concerns_the_decision(uchar x, uchar y) const {
        return x > y? concerns_the_decision(y, x):
            decision.x() == x && decision.y() == y;
    }
    
    inline bool edges(uchar x, uchar y) const {
        if(concerns_the_decision(x, y))
            return decision.is_TH(x, y);
        return graph.edges(x, y);
    }

    inline bool biedges(uchar x, uchar y) const {
        if(concerns_the_decision(x, y))
            return decision.is_HH();
        return graph.biedges(x, y);
    }
    
    bool apply_detailed_analysis(Bits*);
    void construct_bottleneck(Bits*);
    
    bool could_be_collider(uchar) const;
    bool could_be_noncollider(uchar) const;
    
    bool revisits_are_justified(Bits*, uchar, Bits, Bits);
    
    bool matches_any_irrelevancy_rule();    
    size_t pointer_count(uchar) const;

    bool inspect_trails(std::vector<BottleNode>&, uchar, uchar, Bits, Bits);
    
private:
    Completion graph;
    uchar source, target;
    Edge decision;
    size_t iterations;
    std::vector<BottleNode> bottles[2];
    GraphEvaluator::Bottleneck unavoidability;
};

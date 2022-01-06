#include <RelevancyTester.hpp>

#define N graph.size()

bool RelevancyTester::relevant_for(uchar s, uchar t) {
    source = s;
    target = t;
    
    iterations = 0;
    unavoidability = GraphEvaluator::Bottleneck();

    if(!graph.requires(DisableRelevancyRules) && matches_any_irrelevancy_rule())
        return false;
    
    return !graph.requires(RelevancyAlgorithm) || inspect_trails();
}

bool RelevancyTester::relevant_v_structure(uchar node) {
    size_t count = 0;
    for(uchar v = 0; v < graph.size(); v++) {
        if(!relevant_neighbour(node, v))
            continue;
        if(concerns_the_decision(v, node) && (decision.is_HH() || node == decision.effect()))
            count++;
        else if(graph.edges(v, node) || graph.biedges(v, node))
            count++;
    }
    return count >= 2;
}

bool RelevancyTester::revisits_are_justified(Bits* bnecks, uchar node,
        Bits descendants, Bits revisits) {
    iterations++;
    descendants.set(node);
    
    if((descendants & revisits) == revisits) {
        Bits visits = Bits().set(N);
        return effect_could_be_collider(node, visits);
    }
    
    for(uchar ancestor = 0; ancestor < N; ancestor++) {
        if(descendants.test(ancestor)) continue;
        //if(!revisits.test(ancestor)) continue;
        if(!edges(ancestor, node)) continue;
        
        if(revisits_are_justified(bnecks, ancestor, descendants, revisits))
            return true;
    }
    return false;
}

bool RelevancyTester::could_be_collider(uchar node) const {
    
    uchar bidir = 0, causes = 0;
    if(graph.type() == MAXIMAL) {
        if(decision.is_HH() && (node == decision.x() || node == decision.y()))
            bidir++;
    
        if(!decision.is_HH() && node == decision.effect())
            causes++;
    }
    
    for(uchar neigh = 0; neigh < N; neigh++)
        if(relevant_neighbour(node, neigh)) {
            if(biedges(neigh, node)) bidir++;
            if(edges(neigh, node)) causes++;
        }
    return bidir + causes > 1;
}

bool RelevancyTester::could_be_noncollider(uchar node) const {
    if(!decision.is_HH() && node == decision.cause())
        return true;

    for(uchar neigh = 0; neigh < N; neigh++)
        if(edges(node, neigh))
            return true;
    return false;
}

bool RelevancyTester::apply_detailed_analysis(Bits* bnecks) {
    Bits chain = bnecks[0] & bnecks[1];
    
    if(chain.any()) {
        if(decision.is_HH() || !revisits_are_justified(bnecks, decision.cause(), Bits(), chain))
            return false;
    }
 
    for(uchar node = 0; node < N; node++) {
        if(node == target || node == source || chain.test(node))
            continue;
        if(!(bnecks[0] | bnecks[1]).test(node) && node != decision.x() && node != decision.y())
            continue;
        if(!could_be_collider(node) && (chain.none() || node != decision.effect()))
            unavoidability.noncolliders.set(node);
        if(!could_be_noncollider(node))
            unavoidability.colliders.set(node);
    }
    
    for(uchar node = 0; node < N; node++) {
        if(!chain.test(node)) continue;
        
        if(could_be_noncollider(node))
            unavoidability.noncolliders.set(node);
        else
            unavoidability.colliders.set(node);
    }
    
    if(!decision.is_HH() && unavoidability.noncolliders.test(decision.effect())) {
        Bits visited;
        if(effect_could_be_collider(decision.effect(), visited))
            unavoidability.noncolliders.reset(decision.effect());
    }
    
    if(!decision.is_HH())
        unavoidability.noncolliders.set(decision.cause());
    return true;
}

bool RelevancyTester::effect_could_be_collider(uchar node, Bits& visited) {
    iterations++;
    
    if(visited.any() && relevant_v_structure(node))
        return true;
    
    if(visited.test(node))
        return false;
    visited.set(node);
    
    for(uchar ancestor = 0; ancestor < N; ancestor++) {
        if(!relevant_neighbour(node, ancestor)) continue;
        if(!edges(ancestor, node)) continue;

        if(effect_could_be_collider(ancestor, visited))
            return true;
    }
    return false;
}

void RelevancyTester::construct_bottleneck(Bits* bnecks) {
    bnecks[0] = source == decision.x() || source == decision.y()? 0: full_set(N);
    bnecks[1] = target == decision.x() || target == decision.y()? 0: full_set(N);
    
    for(uchar i = 0; i <= 1; i++) {
        bottles[i][decision.x()].relevant_neighbours.set(decision.y());
        bottles[i][decision.y()].relevant_neighbours.set(decision.x());
        
        if(decision.x() != source && decision.x() != target)
            bnecks[i] &= bottles[i][decision.x()].bottlenecks;
        if(decision.y() != source && decision.y() != target)
            bnecks[i] &= bottles[i][decision.y()].bottlenecks;
        
        bnecks[i].reset(source).reset(target);
    }
}

bool RelevancyTester::inspect_trails() {
    Bits targets = Bits().set(decision.x()).set(decision.y());

    bottles[0] = std::vector<BottleNode>(N, BottleNode(N));
    bottles[1] = std::vector<BottleNode>(N, BottleNode(N));
    
    if( !inspect_trails(bottles[0], source, N, Bits(), Bits(targets).reset(target)) ||
        !inspect_trails(bottles[1], target, N, Bits(), Bits(targets).reset(source)))
        return false;
    
    Bits bnecks[2];
    construct_bottleneck(bnecks);
    return apply_detailed_analysis(bnecks);
}

bool RelevancyTester::inspect_trails(std::vector<BottleNode>& bottles, uchar node,
        uchar prev_node, Bits bottleneck, Bits targets) {
    iterations++;
    bottleneck.set(node);
    
    BottleNode& bottle = bottles[node];
    bottle.relevant_neighbours.set(prev_node);
    if(!bottle.is_reduced_by(bottleneck))
        return targets.test(node) || bottle.leads_to_goal;
    bottleneck = bottle.reduce_by(bottleneck);
    
    if(targets.test(node)) return true;
    if(prev_node != N)
        if(node == target || node == source)
            return false;
    
    for(uchar next_node = 0; next_node < N; next_node++) {
        if(!edges(node, next_node) && !edges(next_node, node) && !biedges(node, next_node))
            continue;
        
        if(inspect_trails(bottles, next_node, node, bottleneck, targets)) {
            bottle.relevant_neighbours.set(next_node);
            bottle.leads_to_goal = true;
        }
    }
    return bottle.leads_to_goal;
}

size_t RelevancyTester::pointer_count(uchar var) const {
    size_t count = 0;

    for(uchar v = 0; v < graph.size(); v++) {
        if(concerns_the_decision(v, var) && (decision.is_HH() || var == decision.effect()))
            count++;
        else if(graph.edges(v, var) || graph.biedges(v, var))
            count++;
    }
    return count;
}

bool RelevancyTester::matches_any_irrelevancy_rule() {
    GraphTools tools(graph);
    
    if(decision.absent() && (
            tools.has_inducing_trail(source, target) ||
            tools.has_inducing_trail(target, source)))
        return true;
    
    if(decision.present() && !tools.has_walk_between(source, target))
        return true;

    auto sufficient_connections = [&](uchar node) {
        return node == source || node == target ||
            pointer_count(node) <= 1;
    };
    
    if(decision.is_TH() || decision.is_HT()) {
        if(tools.has_nontrivial_inducing_trail(decision.cause(), decision.effect()))
            return true;

        if(tools.has_inducing_trail(decision.effect(), decision.cause()) &&
            sufficient_connections(decision.cause()) &&
            sufficient_connections(decision.effect())
        ) return true;

    } else if(decision.is_HH()) {
        
        if((tools.has_inducing_trail(decision.x(), decision.y()) && sufficient_connections(decision.x()) ) ||
           (tools.has_inducing_trail(decision.y(), decision.x()) && sufficient_connections(decision.y()) )) {
            return true;
        }
    }
    return false;
}

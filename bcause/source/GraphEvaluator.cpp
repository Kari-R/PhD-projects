#include <GraphEvaluator.hpp>
#include <RelevancyTester.hpp>

#define N instance.size()

GraphEvaluator::GraphEvaluator(const Instance& instance, NodeStats& stats, CPLEX* cplex, CoreSet* cores):
    instance(instance), stats(stats), cplex(cplex), cores(cores) {
    
    open_scope(0);
}

void GraphEvaluator::set(const Constraint* c, bool status) {
    if(cplex != nullptr)
        cplex->set(*c, status);
    
    if(cores != nullptr) {
        if(status == CPLEX::Sat)
            cores->mark_satisfied(*c);
        else
            cores->mark_unsatisfied(*c);
    }
    
    removed.back().push_back(c);
    stats.mark<Status::Removed>(*c);
    statuses.insert({c->ident(), status});
    if(status == CPLEX::Unsat)
        min_weight.back() += c->weight;
}

Weight GraphEvaluator::naive_lb(const Graph& graph) const {
    GraphTools min_comp(graph, MINIMAL);
    GraphTools max_comp(graph, MAXIMAL);
    Weight weight = 0;

    for(const Constraint& c: instance.independent)
        if(min_comp.has_active_trail(c)) {
            if(status(c) == CPLEX::Undefined) print("[independence] Undefined? %\n", c);
            weight += c.weight;
        }
    
    for(const Constraint& c: instance.dependent)
        if(!max_comp.has_active_trail(c)) {
            if(status(c) == CPLEX::Undefined) print("[dependence] Undefined? %\n", c);
            weight += c.weight;
        }
    return weight;
}

GraphEvaluator::Bottleneck GraphEvaluator::naive_bottleneck(const Completion& graph, uchar var, uchar prev, Bits endpoints, const Edge& decided) const {
    
    Bottleneck bneck;

    uchar into_var = 0;
    uchar out_from_var = 0;
    uchar next_count = 0;
    uchar next = graph.size();

    for(uchar neigh = 0; neigh < graph.size(); neigh++) {

        bool has_th = graph.edges(var, neigh)   || decided.is_TH(var, neigh);
        bool has_ht = graph.edges(neigh, var)   || decided.is_TH(neigh, var);
        bool has_hh = graph.biedges(var, neigh) || decided.is_HH(var, neigh);
        
        if(has_th)
            out_from_var++;
        
        if(has_ht || has_hh)
            into_var++;
        
        if(neigh != prev && (has_th || has_ht || has_hh)) {
            next_count++;
            next = neigh;
        }
    }

    if(!endpoints.test(var)) {
        if(!out_from_var) bneck.colliders.set(var);
        if(into_var <= 1) bneck.noncolliders.set(var);
    }
    
    if(next_count != 1 || endpoints.test(next))
        return bneck;
    
    if(!graph.edges(var, next) && !graph.biedges(var, next))
        bneck.noncolliders.set(next);
    
    bneck |= naive_bottleneck(graph, next, var, endpoints, decided);
    return bneck;
}

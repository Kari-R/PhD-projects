#include <ObjectiveFunction.hpp>
#include <RelevancyTester.hpp>

#define N instance.size()

ObjectiveFunction::ObjectiveFunction(const Instance& instance, NodeStats& stats, CPLEX* cplex, CoreSet* cores):
    GraphEvaluator(instance, stats, cplex, cores), lists(N * N, List()), used_time(0) {
    
    for(const Constraint& c: instance.independent)
        lists[c.v1 + c.v2 * N].push_back( &c );
    
    for(const Constraint& c: instance.dependent)
        lists[c.v1 + c.v2 * N].push_back( &c );
}

void ObjectiveFunction::update_list(List& list, const Bottleneck& bneck, GraphTools& tools, bool in_max_completion) {
    if(list.empty())
        return;
    
    for(auto it = list.begin(); it != list.end();) {
        if((((*it)->condition & bneck.colliders) != bneck.colliders) ||
                ((*it)->condition & bneck.noncolliders).any()) {
            it++;
            continue;
        }
        ensure(status(**it) == CPLEX::Undefined);
 
        bool has_trail = tools.has_active_trail(**it);

        if((!in_max_completion && has_trail && (*it)->independence) ||
            (in_max_completion && !has_trail && !(*it)->independence))
            ObjectiveFunction::set(it, list, CPLEX::Unsat);

        else if((!in_max_completion && has_trail && !(*it)->independence) ||
                 (in_max_completion && !has_trail && (*it)->independence))
            ObjectiveFunction::set(it, list, CPLEX::Sat);

        else
            it++;
    }
}

void ObjectiveFunction::update(const Graph& graph, const Edge& decision) {
    size_t start_time = get_time();
    open_scope(min_weight.back());

    Completion completion(graph, decision.absent()? MAXIMAL: MINIMAL,
        [&](Edge edge) {
            if(instance[Acyclicity]) {
                if(edge.is_TH() && graph.has_directed_path(edge.y(), edge.x())) return false;
                if(edge.is_HT() && graph.has_directed_path(edge.x(), edge.y())) return false;
            }
            return (cores != nullptr && cores->induced_independence(edge.x(), edge.y())) ||
                  (cplex != nullptr && cplex->induced_independence(edge.x(), edge.y()));
    });
    
    RelevancyTester decision_eval(completion, decision);
    GraphTools tools(completion);

    for(uchar y = 1; y < N; y++)
        for(uchar x = 0; x < y; x++) {
            if(!stats.total_weight(x, y)[0] && !stats.total_weight(x, y)[1])
                continue;

            if(!decision_eval.relevant_for(x, y))
                continue;

            Bottleneck bneck = decision_eval.bottleneck();
            Bits endpoints; endpoints.set(x); endpoints.set(y);
            
            if(!graph.requires(DisableRelevancyRules)) {
                bneck |= naive_bottleneck(completion, x, N, endpoints, decision);
                bneck |= naive_bottleneck(completion, y, N, endpoints, decision);
            }

            if(decision.absent()) {
                bneck.colliders.reset(decision.x()).reset(decision.y());
                bneck.noncolliders.reset(decision.x()).reset(decision.y());
            }
            update_list(lists[x + y * N], bneck, tools, decision.absent());
        }

#if DEBUG
//    if(!instance[RCFixing]) {
//        Weight naive = naive_lb(graph);
//        ideal_eq(min_weight.back(), ==, naive, {
//            print("Decision: %\n", decision.string());
//            print("Graph:\n");
//            graph.output();
//        });
//    }
#endif
    used_time += get_time() - start_time;
}

Weight ObjectiveFunction::calculate_final_weight(const Graph& graph) const {
    Weight weight = min_weight.back();

    GraphTools finder(graph, MINIMAL);
    
#if DEBUG
    Weight naive_dep = 0, naive_indep = 0;
  
    for(const Constraint& c: instance.independent)
        if(finder.has_active_trail(c))
            naive_indep += c.weight;
    
    for(const Constraint& c: instance.dependent)
        if(!finder.has_active_trail(c))
            naive_dep += c.weight;
#endif
    
    for(uchar y = 1; y < N; y++)
        for(uchar x = 0; x < y; x++) {
            if(!stats.total_weight(x, y)[0] && !stats.total_weight(x, y)[1])
                continue;
            
            for(const Constraint* c: lists[x + y * N]) {
                bool has_trail = finder.has_active_trail(*c);
                if(has_trail == c->independence)
                    weight += c->weight;
            }
        }

    ensure_eq(weight, ==, naive_dep + naive_indep);
    return weight;
}

void ObjectiveFunction::set(List::iterator& it, List& list, bool status) {
    GraphEvaluator::set(*it, status);
    it = list.erase(it);
}

bool ObjectiveFunction::reset() {
    if(removed.empty())
        return false;

    while(!removed.back().empty()) {
        const Constraint* c = removed.back().back();
        
        stats.mark<Status::Returned>(*c);
        statuses.erase(c->ident());
        removed.back().pop_back();
        lists[c->v1 + c->v2 * N].push_back(c);

        if(cplex != nullptr) cplex->reset(*c);
        if(cores != nullptr) cores->reset(*c);
    }
    close_scope();
    return true;
}

void ObjectiveFunction::clear_progress() {
    for(; reset(); );
    open_scope(0);
    used_time = 0;
}

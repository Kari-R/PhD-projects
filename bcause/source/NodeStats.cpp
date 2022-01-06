#include <NodeStats.hpp>

#define UPDATE_STATS(s) \
  { if(c.independence) {\
        s.max_indep = std::max(s.max_indep, c.weight);\
        s.min_indep = std::min(s.min_indep, c.weight);\
        s.indep_count++;\
    } else {\
        s.max_dep = std::max(s.max_dep, c.weight);\
        s.min_dep = std::min(s.min_dep, c.weight);\
        s.dep_count++;\
    }\
  }

NodeStats::NodeStats(const Instance& instance):
    instance(instance),
    nodes(instance.size(), {0, 0}),
    pairs(instance.size(), std::vector<std::vector<Weight> >(instance.size(), {0, 0})),
    potential_deps(instance.size(), 0) {
        
    nonexistent_extremas.max_indep = 0;
    nonexistent_extremas.max_dep = 0;

    initialize(instance.independent);
    initialize(instance.dependent);
    
    for(uchar y = 1; y < instance.size(); y++)
        for(uchar x = 0; x < y; x++) {
            double dep =   total_weight(x, y)[0]; if(!dep)   dep = 1;
            double indep = total_weight(x, y)[1]; if(!indep) indep = 1;
            
            if(dep / indep >= 10) {
                potential_deps[x].set(y);
                potential_deps[y].set(x);
            }
        }
}

void NodeStats::evaluate_solution(const Graph& graph) const {
    header("Evaluating solution\n");

    for(uchar y = 1; y < instance.size(); y++)
        for(uchar x = 0; x < y; x++) {
            bool edge = graph.edges(x, y) || graph.edges(y, x) || graph.biedges(x, y);
            
            double dep =   total_weight(x, y)[0] / 100; if(!dep)   dep = 1;
            double indep = total_weight(x, y)[1] / 100; if(!indep) indep = 1;

            print("Pair (%, %) %: dependence weight %; independence weight %; ratio: % %\n",
                (int)x, (int)y, edge? "has edge": "no edge",
                        total_weight(x, y)[0], total_weight(x, y)[1],
                        (int)(dep / indep * 100));
        }
}

void NodeStats::initialize(const std::vector<Constraint>& list) {
    if(scopes.empty())
        scopes.push_back(Scope(instance));
    
    for(const Constraint& c: list) {
        pairs[c.v1][c.v2][c.independence] += c.weight;
        nodes[c.v1][c.independence] += c.weight;
        nodes[c.v2][c.independence] += c.weight;
        
        update_extremas(c);
    }
}

void NodeStats::update_extremas(const Constraint& c) {
    ensure(!scopes.empty());
    
    UPDATE_STATS(scopes.back().nodes[c.v1]);
    UPDATE_STATS(scopes.back().nodes[c.v2]);

    auto it = scopes.back().pairs.find(c.v1 + c.v2 * instance.size());
    if(it == scopes.back().pairs.end())
        it = scopes.back().pairs.insert(scopes.back().pairs.begin(),
            std::make_pair(c.v1 + c.v2 * instance.size(), Extremas()));

    UPDATE_STATS(it->second);
}
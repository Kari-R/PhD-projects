#include <Graph.hpp>
#include <algorithm>
#include <random>

#define verbose(...) {}//print(__VA_ARGS__)

score_t Graph::score() const {
    if(status != UNKNOWN)
        return cached_score;

    std::vector<var_t> ordering;
    size_t t = get_time();
    find_lex_order(ordering);
    Graph::ordering_time() += get_time() - t;
    t = get_time();
    evaluate(ordering);
    Graph::scoring_time() += get_time() - t;

    if(status != FEASIBLE)
        cached_score = -INF;
    return cached_score;
}

score_t Graph::get_score(var_t var, const Bitset& parents) const {
    Clique clique;
    for(var_t v: parents.contents())
        clique.set(v);
    ensure(!clique.test(var));
    score_t s1 = instance->get(clique);
    clique.set(var);
    score_t s2 = instance->get(clique);
    return s2 - s1;
}

bool Graph::is_feasible() const {
    if(status == UNKNOWN) {
        std::vector<var_t> ordering;
        find_lex_order(ordering);
        evaluate(ordering);
    }
    return status == FEASIBLE;
}

void Graph::evaluate(const std::vector<var_t>& ordering) const {
    try {
        cached_score = 0;
        std::vector<Bitset> parents(size(), Bitset());
        Bitset preceding;

        parents[ordering[0]].set(ordering[0]);
        preceding.set(ordering[0]);
        cached_score += get_score(ordering[0], Bitset());

        for(var_t i = 1; i < size(); i++) {
            var_t var = ordering[i];

            parents[var] = adjacency[var] & preceding;
            if(parents[var].count() >= instance->max_clique_size()) {
                verbose("%'s parents [%] over treewidth bound %\n", var, parents[var], instance->max_clique_size());
                cached_score = -INF;
                status = TOO_WIDE;
                return;
            }

            cached_score += get_score(var, parents[var]);
            preceding.set(var);

            for(long j = i - 1; j >= 0; j--) {
                var_t parent = ordering[j];
                if(!adjacency[var].test(parent))
                    continue;
                if(!parents[var].subset_of( parents[parent] )) {
                    verbose("Parents of var %: [%]\n", var, parents[var]);
                    verbose("Parents of parent %: [%]\n", parent, parents[parent]);
                    cached_score = -INF;
                    status = NOT_CHORDAL;
                    return;                
                }
                break;
            }
            parents[var].set(var);
        }
        status = FEASIBLE;
        
    } catch(Scorer::OverflowException e) {
        cached_score = -INF;
        status = BDEU_OVERFLOW;
    }
}

void Graph::extract_cliques(std::vector<Bitset>& cliques) const {
    std::vector<var_t> ordering;
    find_lex_order(ordering);
    
    cliques.push_back( Bitset().set(ordering[0]) );

    Bitset preceding;
    preceding.set(ordering[0]);

    for(var_t i = 1; i < size(); i++) {
        var_t var = ordering[i];
        Bitset clique = adjacency[var] & preceding;
        clique.set(var);
        preceding.set(var);
        cliques.push_back(clique);
    }
    
    for(auto it = cliques.begin(); it != cliques.end();) {
        bool is_subset = false;
        for(auto jt = cliques.begin(); jt != cliques.end(); jt++)
            if(!(*it == *jt) && it->subset_of(*jt)) {
                is_subset = true;
                break;
            }
        if(is_subset)
            it = cliques.erase(it);
        else
            it++;
    }
}

var_t Graph::pick_next_variable(const std::list<Bitset>& sets, const Bitset& visited) const {
    auto it = sets.begin();
    ensure(!it->subset_of(visited))

    verbose("Consider [%]\n", *it);
    for(var_t var: it->contents())
        if(!visited.test(var))
            return var;
    ensure(false);
}

void Graph::partition_sets(std::list<Bitset>& sets, var_t var, const Bitset& neighs) const {
    for(auto jt = sets.begin(); jt != sets.end(); jt++) {
        jt->reset(var);
        Bitset mutual = *jt & neighs;
        verbose("  Partitioning [%] where intersection with neighbours [%] is [%]\n", *jt, neighs, mutual);

        for(var_t neigh: mutual.contents())
            jt->reset(neigh);

        if(mutual.any()) {
            jt = sets.insert(jt, mutual);
            jt++;
        }
    }

    for(auto jt = sets.begin(); jt != sets.end();)
        if(!jt->any())
            jt = sets.erase(jt);
        else
            jt++;
}

var_t Graph::find_next_lex_variable(std::list<Bitset>& sets, Bitset& visited) const {
    verbose("Sets:");
    for(auto jt = sets.begin(); jt != sets.end(); jt++)
        verbose(" [%]", *jt);
    verbose("\n");

    var_t var = pick_next_variable(sets, visited);
    verbose("Chose %\n", var);
    visited.set(var);

    Bitset neighs;
    for(var_t neigh: adjacency[var].contents())
        if(!visited.test(neigh))
            neighs.set(neigh);

    partition_sets(sets, var, neighs);
    verbose("====================\n\n");
    return var;
}

void Graph::find_lex_order(std::vector<var_t>& ordering) const {
    Bitset visited;
    std::list<Bitset> sets;

    sets.push_back(Bitset());
    for(var_t i = 0; i < size(); i++)
        sets.back().set(i);

    for(var_t i = 0; i < size(); i++)
        ordering.push_back(find_next_lex_variable(sets, visited));
}

void Graph::init_rand() {
    cached_score = INF;
    status = UNKNOWN;
    adjacency = std::vector<Bitset>(size(), Bitset());

    std::vector<size_t> vars;
    for(size_t var = 0; var < instance->max_size(); var++) {
        vars.push_back(var);
        if(var > 0) {
            size_t i = Rand::get(0, vars.size());
            if(i != vars.size())
                std::swap(vars[i], vars.back());
        }
    }
    
    size_t i = 0;
    while(i < vars.size()) {
        size_t max = std::min(instance->max_clique_size(), vars.size() - i);
        if(max > 3) max = 3;
        size_t size = Rand::get(1, max + 1);
        Bitset clique;
        for(size_t j = 0; j < size; i++, j++)
            clique.set(vars[i]);
        add(clique);
    }
}

struct WeightedEdge {
    size_t x, y;
    score_t weight;
    
    bool operator<(const WeightedEdge& edge) const {
        if(weight != edge.weight)
            return weight > edge.weight;
        if(x != edge.x) return x < edge.x;
        return y < edge.y;
    }
};

struct DisjointNode {
    DisjointNode* parent;
    size_t id;
    
    DisjointNode(size_t id): parent(this), id(id) {}
    
    DisjointNode* root() {
        if(parent != this)
            parent = parent->root();
        return parent;
    }
    
    size_t ident() {
        return root()->id;
    }
    
    static void combine(DisjointNode* a, DisjointNode* b) {
        if(a->root() != b->root())
            b->root()->parent = a->root();
    }
};

void Graph::init_tree() {
    cached_score = INF;
    status = UNKNOWN;
    adjacency = std::vector<Bitset>(size(), Bitset());
    
    std::set<WeightedEdge> edges;
    for(var_t x = 0; x < instance->max_size(); x++) {
        score_t score = get_score(x, Bitset());
        for(var_t y = 0; y < instance->max_size(); y++)
            if(x != y)
                edges.insert({x, y, get_score(x, Bitset().set(y)) - score});
    }
    
    std::vector<DisjointNode*> trees;
    trees.reserve(instance->max_size());
    for(size_t var = 0; var < instance->max_size(); var++)
        trees.push_back(new DisjointNode(var));
    
    for(const WeightedEdge& edge: edges) {
        if(edge.weight < 0) break;
        if(trees[edge.x]->ident() == trees[edge.y]->ident())
            continue;
        
        DisjointNode::combine(trees[edge.x], trees[edge.y]);
        add_edge(edge.x, edge.y);
    }
    
    while(!trees.empty()) {
        delete trees.back();
        trees.pop_back();
    }
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    std::vector<Bitset> cliques;
    graph.extract_cliques(cliques);
    std::vector<std::string> strings;
    for(const Bitset& clique: cliques)
        strings.push_back(to_string(clique));
    std::sort(strings.begin(), strings.end());
    for(var_t i = 0; i < strings.size(); i++) {
        if(i > 0) os << "|";
        os << strings[i];
    }
    return os;
}

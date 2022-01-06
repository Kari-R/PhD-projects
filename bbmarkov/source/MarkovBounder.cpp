#include <MarkovBounder.hpp>
#include <algorithm>

template<bool MIN, typename T>
bool is_reducable(T& data_struct, typename T::iterator& it) {
    auto it2 = it; it2++;
    bool is = false;
    while(it2 != data_struct.end()) {
        bool remove_it, remove_it2;
        if(MIN) {
            remove_it = (*it2 & *it) == *it2;
            remove_it2 = (*it2 & *it) == *it;
        } else {
            remove_it = (*it2 & *it) == *it;
            remove_it2 = (*it2 & *it) == *it2;
        }
        if(remove_it2)
            it2 = data_struct.erase(it2);
        else {
            if(remove_it) is = true;
            it2++;
        }
    }
    return is;
}

template<bool MIN, typename T>
void reduce(T& data_struct) {
    for(auto it = data_struct.begin(); it != data_struct.end();)
        if(is_reducable<MIN>(data_struct, it))
            it = data_struct.erase(it);
        else
            it++;
}

MarkovBounder::MarkovBounder(const Solution& f, Instance& i, uint mv):
    instance(i),
    N(mv),
    bayes_solver(instance, mv),
    state(f) {}

size_t MarkovBounder::best_final_parentset(uint var) const {
    size_t pset = instance.domain_lookup[var].at(0);
    for(uint parent = 0; parent < N; parent++) {
        if(!state.contains(parent))
            continue;
        auto clique = instance.domains[parent][state[parent].parentset].content;
        clique.set(parent);
        size_t candidate = instance.score_tree[var].get(clique);
        if(instance.domains[var][pset].score < instance.domains[var][candidate].score)
            pset = candidate;
    }
    return pset;
}

size_t MarkovBounder::find_best_relaxedly_moral_parentset(const Cliques* cliques, uint var, const std::bitset<32>& assigned) {
    
    size_t best = instance.score_tree[var].get(0);
    for(const std::bitset<32>& clique: cliques[var]) {
        auto relaxed_clique = clique | (assigned & ~state.assign_mask);

        size_t pset = instance.score_tree[var].get(relaxed_clique);
        if(instance.domains[var][best].score < instance.domains[var][pset].score)
            best = pset;
    }
    return best;
}

Score MarkovBounder::upperbound(const Cliques* cliques, std::bitset<32> assigned,
        size_t remaining_depth, Score tail_score, Score lb) {

    if(!remaining_depth)
        return bayes_solver.solve(assigned);

    Score ub = -INF;
    for(uint var = 0; var < N; var++) {
        if(assigned.test(var))
            continue;

        size_t pset = find_best_relaxedly_moral_parentset(cliques, var, assigned);
        
        assigned.set(var);
        auto it = seen.find(assigned);
        Score head_score = instance.domains[var][pset].score +
            (it != seen.end()? it->second:
                upperbound(cliques, assigned, remaining_depth - 1,
                    tail_score + instance.domains[var][pset].score, lb));
 
        if(lb < tail_score + head_score) return head_score;
        if(ub < head_score) ub = head_score;
        
        assigned.reset(var);
    }
    seen.insert(std::make_pair(assigned, ub));
    return ub;
}

void MarkovBounder::construct_naive_cliques(Cliques& cliques) {
    for(uint var = 0; var < N; var++) {
        if(!state.contains(var))
            continue;
        cliques.push_back(instance.domains[var][state[var].parentset].content);
        cliques.back().set(var);
    }
    reduce<false>(cliques);
}

void MarkovBounder::decompose_clique(Cliques& tight_cliques, std::bitset<32> clique,
    const Cliques& bad_cliques, size_t i, std::unordered_set<std::bitset<32> >* visited) {    
    if(i >= bad_cliques.size()) {
        tight_cliques.push_back(clique);
        return;
    }
    if(visited[i].find(clique) != visited[i].end())
        return;
    visited[i].insert(clique);
    if((bad_cliques[i] & clique) != bad_cliques[i]) {
        decompose_clique(tight_cliques, clique, bad_cliques, i + 1, visited);
        return;
    }
    for(uint var = 0; var < N; var++) {
        if(!bad_cliques[i].test(var))
            continue;
        clique.reset(var);
        decompose_clique(tight_cliques, clique, bad_cliques, i + 1, visited);
        clique.set(var);
    }
}

void MarkovBounder::construct_tight_cliques(Cliques& tight_cliques,
        const Cliques& bad_cliques, const Cliques& naive_cliques) {
    
    std::unordered_set<std::bitset<32> > visited[bad_cliques.size()];
    for(const std::bitset<32>& clique: naive_cliques)
        decompose_clique(tight_cliques, clique, bad_cliques, 0, visited);
    reduce<false>(tight_cliques);
}

std::string debug(const std::bitset<32>& bs) {
    std::string s;
    for(uint v = 0; v < 32; v++) {
        if(!bs.test(v)) continue;
        if(s != "") s += ", ";
        s += to_string(v);
    }
    return s;
}

#define ULTRA_TIGHT 0

Score MarkovBounder::tight_upperbound(size_t max_depth, Score lb) {
    if(max_depth > N - state.assign_mask.count())
        max_depth = N - state.assign_mask.count();

    Cliques cliques[N];
    for(uint var = 0; var < N; var++) {
        if(state.contains(var))
            continue;
        
        construct_naive_cliques(cliques[var]);
        if(!ULTRA_TIGHT) continue;
        
        Cliques bad_cliques;
        find_bad_cliques(var, bad_cliques);
        
        if(bad_cliques.size() != 0) {
            Cliques tight_cliques;
            construct_tight_cliques(tight_cliques, bad_cliques, cliques[var]);
            cliques[var].swap(tight_cliques);
        }
    }
    
    seen.clear();
    return state.score + upperbound(cliques, state.assign_mask, max_depth, state.score, lb);
}

Network MarkovBounder::upperbound_solution() {
    ArcMatrix matrix(N, std::bitset<32>());
    for(uint var = 0; var < N; var++) {
        if(!state.contains(var))
            continue;
        for(uint parent: instance.domains[var][state[var].parentset].list)
            matrix[var][parent] = true;
        matrix[var][var] = true;
    }
    
    bayes_solver.construct_solution(matrix, state.assign_mask);
    return Network { matrix, state.score + bayes_solver.solve(state.assign_mask) };
}

Network MarkovBounder::lowerbound_solution(size_t branching) {
    Network ub = upperbound_solution();
    return Moralizer(instance, N).moralize(ub.matrix, branching);
}

bool MarkovBounder::has_path_to(uint source, uint target, std::bitset<32>& has_path, std::bitset<32>& no_path) {
    if(source == target || has_path.test(source)) return true;
    if(no_path.test(source)) return false;
    
    for(uint parent: instance.domains[source][state[source].parentset].list)
        if(has_path_to(parent, target, has_path, no_path)) {
            has_path.set(source);
			return true;
		}
    no_path.set(source);
    return false;
}

void MarkovBounder::find_bad_cliques(uint var, Cliques& bad_cliques) {
    for(uint target = 0; target < N; target++) {
        if(!state.contains(target) || target < var)
            continue;
        
        std::bitset<32> has_path, no_path;
        for(uint source = 0; source < N; source++) {
            if(!state.contains(source) || (target != source &&
                    !has_path_to(source, target, has_path, no_path)))
                continue;
            
            std::bitset<32> subset = instance.domains[target][state[target].parentset].content;
            subset.set(target);
            bad_cliques.push_back(subset);
        }
    }

    reduce<true>(bad_cliques);
}
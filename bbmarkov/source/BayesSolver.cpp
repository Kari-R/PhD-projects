#include <BayesSolver.hpp>
#include <debug.hpp>
#include <algorithm>

BayesSolver::BayesSolver(Instance& i, uint mv):
    instance(i), N(mv) {
    scores.insert(scores.begin(), size_t(1) << N, -INF);
    scores[full_set(N).to_ulong()] = 0;
}

size_t BayesSolver::find_best_parentset(uint var, const std::bitset<32>& assigned) const {
    return instance.score_tree[var].get(assigned);
}

Score BayesSolver::solve(std::bitset<32> assigned) {

    Score& value = scores[assigned.to_ulong()];
    if(value != -INF)
        return value;

    for(uint var = 0; var < N; var++) {
        if(assigned[var]) continue;
        
        assigned[var] = true;
        Score score = solve(assigned);
        assigned[var] = false;
        
        score += instance.score_tree[var].best_within(assigned).score;
        if(value < score)
            value = score;
    }
    return value;
}

uint BayesSolver::find_best_variable(std::bitset<32> assigned) const {
    uint best_var = 0;
    Score best_score = -INF;
    for(uint var = 0; var < N; var++) {
        if(assigned[var]) continue;
        assigned[var] = true;

        size_t pset = find_best_parentset(var, assigned);
        Score score = scores[assigned.to_ulong()] + instance.domains[var][pset].score;
        if(score > best_score) {
            best_var = var;
            best_score = score;
        }
        assigned[var] = false;
    }
    return best_var;
}

void BayesSolver::assign_next_variable(ArcMatrix& matrix, std::bitset<32>& assigned) const {
    
    uint var = find_best_variable(assigned);
    size_t pset = find_best_parentset(var, assigned);
    
    for(uint parent: instance.domains[var][pset].list)
        matrix[var][parent] = true;
    matrix[var][var] = true;
    assigned[var] = true;
}

void BayesSolver::construct_solution(ArcMatrix& matrix, std::bitset<32> assigned) const {
    assert(scores[assigned.to_ulong()] != -INF);

    while(assigned.count() < N)
        assign_next_variable(matrix, assigned);
}
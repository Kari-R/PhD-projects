#include <Moralizer.hpp>
#include <debug.hpp>

#define N max_var

Moralizer::Moralizer(Instance& i, uint mv):
    instance(i),
    max_var(mv) {}

std::bitset<32> Moralizer::parentset_of(const ArcMatrix& graph, uint var) {
    std::bitset<32> content = graph[var];
    content[var] = false;
    return content;
}

void Moralizer::consider_edge_addition(Queue& queue, Network network, uint source, uint target) {
    std::bitset<32> pset = parentset_of(network.matrix, target);
    network.score -= instance.domains[target][instance.domain_lookup[target].at(pset)].score;
    
    network.matrix[target][source] = true;
    if(network.is_within_cycle(source))
        return;
    
    pset = parentset_of(network.matrix, target);
    if(pset.count() > instance.max_pset)
        return;
    
    network.score += instance.domains[target][instance.domain_lookup[target].at(pset)].score;
    queue.push(network);
}

void Moralizer::consider_edge_removal(Queue& queue, Network network, uint child, uint parent) {
    std::bitset<32> pset = parentset_of(network.matrix, child);
    network.score -= instance.domains[child][instance.domain_lookup[child].at(pset)].score;
    
    network.matrix[child][parent] = false;
    pset = parentset_of(network.matrix, child);
    network.score += instance.domains[child][instance.domain_lookup[child].at(pset)].score;
    queue.push(network);
}

size_t Moralizer::find_immoralities(Queue& queue, const Network& network, Direction direction) {

    size_t count = 0;
    std::unordered_set<size_t> pairs;
    for(uint var = 0; var < N; var++) {
        for(uint p1 = 0; p1 < N; p1++) {
            if(!network.matrix[var][p1] || var == p1) continue;
            
            for(uint p2 = 0; p2 < p1; p2++) {
                if(!network.matrix[var][p2] || var == p2) continue;
                if(network.matrix[p1][p2] || network.matrix[p2][p1]) continue;
                if(pairs.find(p1 + p2 * N) != pairs.end()) continue;
                
                pairs.insert(p1 + p2 * N);

                if(direction == AddEdges) {
                    consider_edge_addition(queue, network, p1, p2);
                    consider_edge_addition(queue, network, p2, p1);
                }
                count++;
            }
        }
    }
    
    if(direction == RemoveEdges)
        for(uint var = 0; var < N; var++)
            for(uint p = 0; p < N; p++)
                if(network.matrix[var][p] && var != p)
                    consider_edge_removal(queue, network, var, p);
    return count;
}

void Moralizer::clear_candidates(Queue& queue, size_t accuracy) {
    if(accuracy >= queue.size())
        return;
    std::priority_queue<Network> best;
    while(!queue.empty()) {
        if(best.size() < accuracy)
            best.push(queue.top());
        queue.pop();
    }
    queue.swap(best);
}

void Moralizer::fix_global_immoralities(const Network& network, Network& best,
        Direction direction, size_t accuracy, size_t depth) {
    
    if(network.score <= best.score || visited.find(network.matrix) != visited.end())
        return;
    visited.insert(network.matrix);

    Queue queue;
    if(!find_immoralities(queue, network, direction)) {
        if(best.score < network.score)
            best = network;
        return;
    }
    clear_candidates(queue, depth >= 2? 1: accuracy);

    while(!queue.empty()) {
        fix_global_immoralities(queue.top(), best, direction, accuracy, depth + 1);
        queue.pop();
    }
}

Network Moralizer::moralize(const ArcMatrix& graph, Direction direction, size_t accuracy) {
    Score score = 0;
    for(uint var = 0; var < N; var++) {
        size_t pset = instance.domain_lookup[var].at(parentset_of(graph, var));
        score += instance.domains[var][pset].score;
    }
    
    Network best { ArcMatrix(), -INF };
    visited.clear();
    fix_global_immoralities(Network {graph, score},
        best, direction, accuracy, 0);
    return best;
}

Network Moralizer::moralize(const ArcMatrix& graph, size_t accuracy) {
    Network filled = moralize(graph, Moralizer::AddEdges, accuracy);
    Network reduced = moralize(graph, Moralizer::RemoveEdges, accuracy);

    return filled.score > reduced.score? filled: reduced;
}
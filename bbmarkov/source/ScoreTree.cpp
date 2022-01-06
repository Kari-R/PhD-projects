#include <ScoreTree.hpp>

ScoreTree::ScoreTree(uint size):
N(size), sequences(size_t(1) << N, INF) {}

void ScoreTree::construct(const Domain& d, const DomainLookup& dl) {
    domain = &d;
    domain_lookup = &dl;

    sequences[0] = domain_lookup->at(0);
    construct(full_set(N));
}

size_t ScoreTree::construct(std::bitset<32> subset) {
    size_t& current = sequences[subset.to_ulong()];
    if(current != INF)
        return current;
    
    if(domain_lookup->find(subset) != domain_lookup->end())
        current = domain_lookup->at(subset);
    
    for(uint var = 0; var < N; var++) {
        if(!subset.test(var))
            continue;
        subset.reset(var);
        size_t pset = construct(subset);
        if(current == INF || (* domain)[current].score < (* domain)[pset].score)
            current = pset;
        subset.set(var);
    }
    return current;
}
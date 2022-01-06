#include <Scorer.hpp>
#include <cmath>

#define MAX_Q score_t(size_t(std::numeric_limits<score_t>::max) / 2)

score_t Scorer::operator()(const Clique& clique) {
    auto it = scores.find(clique);
    if(it == scores.end()) {
        if(csv == nullptr) {
            print("Error: Missing clique score for [%]!\n", clique);
            std::exit(-1);
        }
        size_t start_time = get_time();
        std::vector<var_t> vec;
        for(size_t i = 0; i < clique.size(); i++)
            if(clique.test(i))
                vec.push_back(i);
        
        score_t score;
        try {
            score = compute(vec, 0, 1, samples);
        } catch(OverflowException e) {
            computation_time() += get_time() - start_time;
            throw e;
        }
        it = scores.insert(scores.begin(),
            std::make_pair(clique, score));
        computation_time() += get_time() - start_time;
    }
    return it->second;
}

size_t Scorer::precompute(size_t tw) {
    scores.clear();
    size_t start_time = get_time();
    print("Precomputing scores up to treewidth of %...\n", tw);
    compute(Clique(), 0, 1, samples, tw);
    computation_time() += get_time() - start_time;
    print("% clique scores computed (% ms).\n",
        scores.size(), (get_time() - start_time) / 1000);
    return get_time() - start_time;
}

score_t Scorer::compute(const std::vector<var_t>& clique, size_t depth, size_t q,
        const std::vector<size_t>& samples) {
    if(depth >= clique.size())
        return 0;

    var_t var = clique[depth];
    size_t r = csv->arity(var);
    
    score_t score = lgamma(ess/q) - lgamma(samples.size() + ess/q);

    std::vector<std::vector<size_t>> by_value(r);
    for(size_t id: samples)
        by_value[(*csv)[id][var]].push_back(id);

    for(size_t value = 0; value < r; value++) {
        if(by_value[value].empty())
            continue;
        
        if(q >= MAX_Q / r)
            throw OverflowException {q, r};
        score += lgamma(ess/(r*q) + by_value[value].size()) - lgamma(ess/(r*q));
        score += compute(clique, depth + 1, q * r, by_value[value]);
    }
    return score;
}

void Scorer::compute(Clique clique, size_t var, size_t q,
        const std::vector<size_t>& samples, size_t max_tw) {    
    auto it = scores.find(clique);
    if(it == scores.end())
        it = scores.insert(scores.begin(), std::make_pair(clique,
                lgamma(ess/1) - lgamma(this->samples.size() + ess/1)
            ));
    it->second += lgamma(ess/q + samples.size()) - lgamma(ess/q);

    if(var < csv->variables() && clique.count() < max_tw) {
        for(size_t var2 = var; var2 < csv->variables(); var2++) {
            
            size_t r = csv->arity(var2);
            
            std::vector<std::vector<size_t>> by_value(r);
            for(size_t id: samples)
                by_value[(*csv)[id][var2]].push_back(id);
            
            for(size_t value = 0; value < r; value++) {
                if(!by_value[value].empty()) {
                    Clique clique2 = Clique(clique).set(var2);
                    compute(clique2, var2 + 1, q * r, by_value[value], max_tw);
                }
            }
        }
    }
}

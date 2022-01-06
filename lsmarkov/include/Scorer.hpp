#pragma once

#include <CSV.hpp>
#include <unordered_set>
#include <bitset>
#include <Bitset.hpp>

using Bits = std::bitset<256>;

using Clique = Bits;

std::ostream& operator<<(std::ostream&, const Clique&);

#define INF 1000000000

class Scorer {
public:
    struct OverflowException { size_t q, r; };
    
    Scorer(): csv(nullptr) {}
    
    size_t& computation_time() { static size_t t = 0; return t; }

    void to_be_computed_using(const CSV* csv_, score_t ess_) {
        csv = csv_;
        ess = ess_;
        
        samples.clear();
        for(size_t i = 0; i < csv->size(); i++)
            samples.push_back(i);
    }
    
    inline void put(const Clique& clique, score_t score) {
        scores.insert(std::make_pair(clique, score));
    }
    
    size_t precompute(size_t);
    
    score_t operator()(const Clique&);
    
    score_t compute(const std::vector<var_t>&, size_t, size_t, const std::vector<size_t>&);
    void compute(Clique, size_t, size_t, const std::vector<size_t>&, size_t);
    
    inline size_t computed_clique_scores() const { return scores.size(); }
    
private:
    const CSV* csv;
    std::unordered_map<Clique, score_t> scores;
    std::vector<size_t> samples;
    score_t ess;
};

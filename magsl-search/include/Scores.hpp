#pragma once

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <Comparent.hpp>
#include <Config.hpp>

class Scores {
public:
    Scores(std::istream&);

    void get_partitions(std::vector<bits_t>&, bits_t, bits_t, node_t);
    void copy_parents(Comparent::Key&, const Comparent::Key&);
    
    void preprocess(const Config&);
    void output_to(std::ostream&) const;
    
    const std::vector<size_t>& by_component(bits_t) const;
    
    inline const Comparent& get(size_t id) const {
        ensure(id < comparents.size());
        return comparents[id];
    }
    
    inline node_t size() const { return max_vars; }
    inline node_t parent_limit() const { return max_parents; }
    inline node_t component_limit() const { return max_component; }
    
    void gather_subsets(std::vector<bits_t>&, bits_t, bits_t, size_t) const;
    
    score_t get_score(const Comparent::Key& key) const {
        auto it = to_score.find(key);
        if(it == to_score.end())
            return MAX_SCORE;
        return it->second;
    }
    
private:
    node_t max_vars, max_parents, max_component;
    std::vector<Comparent> comparents;
    std::unordered_map<size_t, std::vector<size_t> > by_comp;
    std::vector<std::unordered_map<Comparent::Key, size_t> > map;
    std::unordered_map<Comparent::Key, score_t> to_score;
    size_t iters;
};

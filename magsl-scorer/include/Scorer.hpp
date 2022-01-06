#pragma once

#include <ICF.hpp>
#include <Config.hpp>
#include <Pruner.hpp>

using Scoreables = std::vector<Scoreable>;

class Scorer {
public:
    Scorer(const Config&, ICF*,
        size_t max_pars_per_node = Scoreable::Unlimited,
        size_t max_pars_per_comp = Scoreable::Unlimited,
        std::ostream& out = std::cout,
        size_t max_decimals = 10);
    
    bool is_complete(const Scoreable&) const;

    void iterate_extensions(Scoreable, bits_t);
    size_t output_total_score_count(size_t) const;
    
    void prune_and_output();
    
    inline size_t pruned_score_count() const { return pruner.pruned_score_count(); }
    inline size_t total_prune_time() const { return pruner.total_prune_time(); }
    inline size_t total_score_count() const { return total_count; }
    
    inline std::pair<size_t, size_t> optima_rate_after_pruning() const { return pruner.optima_rate_after_pruning(); }
    inline std::pair<size_t, size_t> optima_rate_before_pruning() const { return pruner.optima_rate_before_pruning(); }

private:
    bool output_missing_edges(const Scoreable&) const;
    void output_parentsets(const Scoreable&) const;
    void output(const Scoreable&, double) const;
    
    void consider(const Scoreable&) const;

    void iterate_extensions(const Scoreable&, const Nodes&, node_t, node_t, size_t);
    
    const Config& config;
    ICF* scorer;
    std::ostream &out;
    const size_t max_pars_per_node, max_pars_per_comp, max_decimals;
    mutable size_t total_count;
    mutable std::unordered_map<size_t, double> complete_scores;
    mutable Pruner pruner;
};

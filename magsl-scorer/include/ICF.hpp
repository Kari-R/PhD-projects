#pragma once

#include <Eigen/Dense>
#include <util.hpp>
#include <iostream>
#include <bitset>
#include <vector>
#include <limits>
#include <unordered_map>
#include <map>

using node_t = unsigned int;
using Nodes = std::vector<node_t>;

struct Scoreable {
    static constexpr size_t Unlimited = -1;
    static constexpr double
        MinScore = -1000000000;
    
    Nodes nodes, outer_parents;
    std::vector<Nodes> parents, spouses;
    size_t edge_count;
    bool is_complete = false, usable_for_pruning = false;
    
    Scoreable(size_t size):
        parents(size, Nodes()),
        spouses(size, Nodes()),
        edge_count(0) {}
    
    inline void add_inner_parent(node_t i, node_t p_i) {
        parents[i].push_back(p_i);
        edge_count++;
    }
    
    inline void add_outer_parent(node_t i, node_t p) {
        if(nodes.back() != p) {
            nodes.push_back(p);
            outer_parents.push_back(nodes.size() - 1);
        }
        parents[i].push_back(nodes.size() - 1);
        edge_count++;
    }

    inline void add_bidir_edge(node_t i, node_t j) {
        spouses[i].push_back(j);
        spouses[j].push_back(i);
        edge_count++;
    }
    
    inline size_t component_size() const {
        return nodes.size() - outer_parents.size();
    }
    
    void sort_spouses() {
        for(node_t v = 0; v < component_size(); v++)
            std::sort(spouses[v].begin(), spouses[v].end());
    }
};

inline std::ostream& operator<<(std::ostream& os, const Scoreable& s) {
    for(node_t i = 0; i < s.component_size(); i++) {
        if(i > 0) os << "; ";
        os << s.nodes[i] << "<-[";
        for(node_t j = 0; j < s.parents[i].size(); j++) {
            if(j > 0) os << ",";
            os << s.nodes[ s.parents[i][j] ];
        }
        os << "]";
    }
    os << " ";
    for(node_t x = 0; x < s.component_size(); x++)
        for(node_t y: s.spouses[x])
            if(x < y)
                os << " " << s.nodes[x] << "<>" << s.nodes[y];
    return os;
}

class ICF {
public:
    struct Stats {
        double highest, lowest, average;
        size_t local_optimas;
    };
    
    ICF(double tolerance = 0.000001):
        tolerance(tolerance), use_rand_start(false), icf_time(0), scored_count(0),
        total_iters(0), most_iters(0) {}

    Eigen::MatrixXd read_data(std::istream&, size_t);
    Eigen::MatrixXd read_covariances(std::istream&, size_t);

    double likelihood(const Scoreable&) const;
    double penalty(const Scoreable&) const;
    double score(const Scoreable&) const;
    
    Stats score(const Scoreable&, size_t, size_t) const;
    
    inline size_t time_spent() const { return icf_time; }
    inline size_t amount_scored() const { return scored_count; }

    inline size_t avg_iterations() const { return size_t(double(total_iters) / scored_count + 0.5); }
    inline size_t max_iterations() const { return most_iters; }
    
    inline size_t variable_count() const { return cov.rows(); }

    size_t MaxIters = Scoreable::Unlimited;

protected:
    Eigen::MatrixXd run_icf(const Eigen::MatrixXd&, const std::vector<Nodes>&, const std::vector<Nodes>&, const Nodes&) const;
    
    void run_icf_iteration(Eigen::MatrixXd&, Eigen::MatrixXd&, const Eigen::MatrixXd&, const std::vector<Nodes>&, const std::vector<Nodes>&) const;
    
    double calc_likelihood(const Eigen::MatrixXd&, const Eigen::MatrixXd&, double, size_t) const;
    
private:
    Eigen::MatrixXd cov;
    double tolerance, sample_count;
    mutable bool use_rand_start;
    mutable size_t icf_time, scored_count, total_iters, most_iters;
};

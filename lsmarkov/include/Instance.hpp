#pragma once

#include <Config.hpp>
#include <util.hpp>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <limits>

#include <Scorer.hpp>

class Instance {
public:
    template<typename T = std::string>
    T read_line(std::ifstream& file) {
        T value;
        std::string line;
        std::getline(file, line);
        std::stringstream stream(line);
        stream >> value;
        return value;
    }
    
    void read_score_file();
    void read_CSV_file();
    
    Instance(const Config&);
    
    inline Config::Arg operator[](size_t arg) const { return config[arg]; }
    
    inline size_t max_size() const { return N; }
    inline size_t max_clique_size() const { return treewidth; }
    
    inline score_t get(Clique clique) const {
        return scores(clique);
    }

    size_t precompute_scores(size_t);
    
    inline std::string name() const { return config.name(); }
    inline const std::string& config_str() const { return config.as_string(); }
    inline size_t score_comp_time() const { return scores.computation_time(); }
    
    inline size_t computed_clique_scores() const { return scores.computed_clique_scores(); }
    
    bool increase_dynamic_treewidth();
    void reset_dynamic_treewidth();
    
private:
    Config config;
    size_t N, treewidth;
    mutable Scorer scores;
};

#include <Instance.hpp>

void Instance::init(std::istream& input) {
    read_cussen_scores(input, domains);
    initialize_lookup_table(domain_lookup, domains);
    score_tree.assign(domains.size(), ScoreTree(domains.size()));
    
    for(size_t i = 0; i < domains.size(); i++)
        score_tree[i].construct(domains[i], domain_lookup[i]);
    
    std::bitset<32> subset;
    for(uint var = 1; var < domains.size(); var++) {
        subset.set(var, true);
        if(domain_lookup[0].find(subset) == domain_lookup[0].end())
            break;
        max_pset++;
    }
}

bool read_till(const std::string& string, size_t& i, std::string& target, char c) {
    for(; i < string.length(); i++) {
        if(string[i] == c) {
            i++;
            return true;
        }
        target += string[i];
    }
    return false;
}

Instance::Instance(const std::string& fname): max_pset(0) {
    std::ifstream file(fname);
    if(file.is_open()) {
        init(file);
        return;
    }
    
    std::string name, vars, pset;
    size_t i = 0;

    if(!read_till(fname, i, name, '_')) return;
    if(!read_till(fname, i, vars, '_')) return;
    if(!read_till(fname + ".", i, pset, '.')) return;
    
    std::ifstream alternative(format("scores/%/%/%_%_%.txt",
        vars, pset, name, vars, pset));
    
    if(alternative.is_open())
        init(alternative);
}
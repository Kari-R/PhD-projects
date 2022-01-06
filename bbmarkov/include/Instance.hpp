#pragma once

#include <ParentSet.hpp>
#include <ScoreTree.hpp>
#include <iostream>

struct Instance {
    std::vector<Domain> domains;
    std::vector<DomainLookup> domain_lookup;
    std::vector<ScoreTree> score_tree;
    size_t max_pset;
    
    void init(std::istream&);
    
    inline bool is_valid() const {
        if(domains.size() == 0)
            return false;
        for(size_t i = 1; i < domains.size(); i++)
            if(domains[i].size() != domains[i - 1].size())
                return false;
        return true;
    }
    
    Instance(const std::string&);
};
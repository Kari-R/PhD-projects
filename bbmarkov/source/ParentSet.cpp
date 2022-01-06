#include <ParentSet.hpp>
#include <sstream>
#include <algorithm>
#include <set>

void initialize_lookup_table(std::vector<DomainLookup>& domain_lookup, const Domains& domains) {
    domain_lookup.reserve(domains.size());
    
    for(uint var = 0; var < domains.size(); var++) {
        domain_lookup.push_back(DomainLookup());
        
        for(size_t pset = 0; pset < domains[var].size(); pset++)
            domain_lookup[var].insert(std::make_pair(domains[var][pset].content, pset));
    }
}

void read_parents(std::istream& input, ParentSet& parents) {
    size_t count = read(input);
    for(size_t i = 0; i < count; i++)
        parents.add(read(input));
}

void read_parentsets(std::istream& input, Domains& domains) {
    size_t variable = read(input);
    size_t count = read(input);

    domains[variable].reserve(count);
    for(size_t i = 0; i < count; i++) {
        domains[variable].push_back(read(input));
        read_parents(input, domains[variable].back());
    }
    
    std::sort(domains[variable].begin(), domains[variable].end(),
    [](const ParentSet& a, const ParentSet& b) {
        if(a.score != b.score)
            return a.score > b.score;
        return a.count() > b.count();
    });
}

void read_cussen_scores(std::istream& input, Domains& data) {
    size_t count = read(input);
    data.reserve(count);
    
    for(size_t i = 0; i < count; i++)
        data.push_back(Domain());
    
    for(size_t i = 0; i < count; i++)
        read_parentsets(input, data);
}

std::ostream& operator<<(std::ostream& os, const ParentSet& parentset) {
    for(size_t i = 0; i < parentset.count(); i++) {
        os << parentset[i];
        if(i + 1 < parentset.count())
            os << ", ";
    }
    return os;
}
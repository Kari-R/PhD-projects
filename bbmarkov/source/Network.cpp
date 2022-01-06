#include <Network.hpp>

bool Network::operator<(const Network& another) const {
    return score < another.score;
}

bool Network::has_path_between(uint var, uint target, std::bitset<32>& visited) {
    if(visited[var])
        return false;
    visited[var] = true;
    for(uint v = 0; v < matrix.size(); v++) {
        if(var == v || !matrix[var][v])
            continue;
        if(v == target || has_path_between(v, target, visited))
            return true;
    }
    return false;
}

bool Network::is_within_cycle(uint var) {
    std::bitset<32> visited;
    return has_path_between(var, var, visited);
}

size_t Network::count_immoralities() const {

    ArcMatrix unsatisfied(matrix.size(), std::bitset<32>());
    for(uint var = 0; var < matrix.size(); var++)
        for(uint parent = 0; parent < matrix.size(); parent++)
            if(var != parent && matrix[var][parent])
                unsatisfied[parent] |= matrix[var];
    
    for(uint var = 0; var < matrix.size(); var++) {
        unsatisfied[var][var] = false;
        for(uint var2 = 0; var2 < var; var2++) {
            if(!matrix[var][var2] && !matrix[var2][var])
                continue;
            unsatisfied[var][var2] = false;
            unsatisfied[var2][var] = false;
        }
    }
    
    size_t count = 0;
    for(uint var = 0; var < matrix.size(); var++)
        count += unsatisfied[var].count();
    return count / 2;
}

size_t NetworkHash::operator()(const ArcMatrix& skeleton) const {
    size_t hash = 0;
    for(const std::bitset<32>& bitset: skeleton)
        hash = hash * 31 + bitset.to_ulong();
    return hash;
}

bool NetworkCompare::operator()(const ArcMatrix& a, const ArcMatrix& b) const {
    assert(a.size() == b.size());
    for(size_t i = 0; i < a.size(); i++)
        if(a[i] != b[i])
            return false;
    return true;
}
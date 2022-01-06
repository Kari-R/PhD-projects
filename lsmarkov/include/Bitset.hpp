#pragma once

#include <util.hpp>
#include <unordered_set>
#include <vector>
#include <algorithm>

using var_t = unsigned short;
using score_t = double;

class Bitset {
public:
    static var_t& capacity() { static var_t x = 32; return x; }
    
    inline Bitset& set() { return *this; }
    
    template<typename... Rest>
    inline Bitset& set(var_t var, Rest... rest) {
        bits[var] = true;
        list.insert(var);
        return set(rest...);
    }
    
    template<typename... Items>
    Bitset(Items... items): bits(capacity(), 0) {
        set(items...);
    }
    
//    inline var_t size() const { return capacity(); }
    inline var_t count() const { return list.size(); }
    inline var_t size() const { return capacity(); }
    
    bool test(var_t var) const { return bits[var]; }
    inline const std::unordered_set<var_t>& contents() const { return list; }

    Bitset operator&(const Bitset& bs) const {
        Bitset bits;
        for(var_t var: bs.contents())
            if(test(var))
                bits.set(var);
        return bits;
    }

    bool subset_of(const Bitset& bs) const {
        if(list.size() > bs.list.size())
            return false;
        for(var_t var: contents())
            if(!bs.test(var))
                return false;
        return true;
    }

    void reset(var_t var) {
        list.erase(var);
        bits[var] = false;
    }
    
    size_t hash() const {
        size_t h = 0;
        for(var_t item: list)
            h += size_t(item) * capacity();
        return h;
    }
    
    bool operator==(const Bitset& bs) const {
        if(count() != bs.count())
            return false;
        return bits == bs.bits;
    }
    
    inline bool any() const { return !list.empty(); }
    
private:
    std::vector<bool> bits;
    std::unordered_set<var_t> list;
};

inline std::ostream& operator<<(std::ostream& out, const Bitset& bitset) {
    std::vector<size_t> items;
    for(var_t var: bitset.contents())
        items.push_back(var);
    std::sort(items.begin(), items.end());
    bool any = false;
    for(var_t var: items) {
        if(any) out << ",";
        out << var;
        any = true;
    }
    return out;
}

namespace std {
    template<>
    struct hash<Bitset> {
        inline bool operator()(const Bitset& bs) const {
            return bs.hash();
        }
    };
}

#pragma once

#include <string>
#include <bitset>
#include <debug.hpp>

typedef std::bitset<32> Bits;

inline Bits full_set(size_t N) {
    return (size_t(1) << N) - 1;
}

inline Bits make_bitset() { return 0; }

template<typename... R>
inline Bits make_bitset(unsigned char val, R... r) {
    Bits bits = make_bitset(r...);
    bits.set(val);
    return bits;
}

inline std::string to_string(Bits bits) {
    if(bits.none())
        return "[]";
    std::string s;
    for(size_t i = 0; i < 32; i++) {
        if(!bits.test(i)) continue;
        if(s != "") s += ",";
        s += to_string(i);
    }
    return s;
}
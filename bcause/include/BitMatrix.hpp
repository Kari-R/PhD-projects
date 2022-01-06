#pragma once

#include <useful.hpp>

class BitMatrix {
public:
    BitMatrix(size_t N): bits(N, 0) {}
    
    inline bool operator()(uchar x, uchar y) const {
        return bits[x].test(y);
    }
    
    inline Bits operator()(uchar x) const { return bits[x]; }
    inline Bits& operator()(uchar x) { return bits[x]; }
    
    inline uchar size() const { return bits.size(); }
    
    inline void set(uchar x, uchar y) { bits[x].set(y); }
    inline void reset(uchar x, uchar y) { bits[x].reset(y); }

private:
    std::vector<Bits> bits;
};

struct SymmetricBitMatrix: public BitMatrix {
public:
    SymmetricBitMatrix(size_t N): BitMatrix(N) {}
    
    inline void set(uchar x, uchar y) {
        BitMatrix::set(x, y);
        BitMatrix::set(y, x);
    }
    
    inline void reset(uchar x, uchar y) {
        BitMatrix::reset(x, y);
        BitMatrix::reset(y, x);
    }
};
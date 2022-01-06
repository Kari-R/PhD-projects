#pragma once

#include <iostream>
#include <sstream>
#include <functional>
#include <iomanip>
#include <chrono>
#include <bitset>
#include <vector>

const size_t FP_PRECISION = 15;

inline void print(const char* fstr) {
    std::cout << fstr << std::flush;
}

#define ABS(x) ((x) < 0? -(x): (x))

template<typename T, typename... L>
void print(const char* fstr, const T& item, L... rest) {
    for(; *fstr; fstr++)
        if(*fstr == '\\') {
            std::cout << *(fstr + 1);
            fstr++;
        }
        else if(*fstr == '%') {
            std::cout << std::setprecision(FP_PRECISION) << item << std::flush;
            print(fstr + 1, rest...);
            break;
        }
        else
            std::cout << *fstr;
    std::cout << std::flush;
}

template<typename T>
T filled_bitset(size_t size) {
    return T((size_t(1) << size) - 1);
}

inline size_t get_time() {
    auto x = std::chrono::system_clock::now().time_since_epoch();
    auto y = std::chrono::duration_cast< std::chrono::microseconds >(x);
    return y.count();
}

#define duration_since(t) (double((get_time() - t)/1000)/1000)

inline std::string capitalize(std::string word) {
    if(word.length() == 0) return word;
    if(word[0] >= 'a' && word[0] <= 'z')
        word[0] += 'A' - 'a';
    return word;
}

#define BENCHMARK(title, ...) \
    { print("%", capitalize(title) + (std::string)"...\n"); size_t tic = get_time(); __VA_ARGS__; print("Done %, took %s\n", title, duration_since(tic)); }

inline std::string ignore_return_type(const std::string& funname) {
    size_t i = 0;
    while(funname[i] != ' ') i++;
    return funname.substr(i + 1, funname.length());
}

#define ensure(x) \
    if(!(x)) {\
        print("[%]\nAssert failed: %\n",\
            ignore_return_type(__PRETTY_FUNCTION__), #x);\
        std::exit(0);\
    }

inline std::vector<std::string> split(const std::string& str, char deli) {
    std::vector<std::string> vec(1, "");
    for(char c: str)
        if(c == deli)
            vec.push_back("");
        else
            vec.back() += c;
    return vec;
}

using bits_t = std::bitset<32>;
using node_t = unsigned short;

class Set {
public:
    inline size_t size() const {
        return vector.size();
    }
    
    inline bits_t bitset() const {
        return bits;
    }
    
    inline bool within(bits_t set) const {
        return (bits & set) == bits;
    }
    
    inline const std::vector<node_t>& contents() const {
        return vector;
    }
    
    inline node_t operator[](node_t i) const {
        ensure(i < size());
        return vector[i];
    }
    
    inline bool has(node_t node) const {
        return bits.test(node);
    }
    
    inline void add(node_t x) {
        if(!bits.test(x)) {
            vector.push_back(x);
            bits.set(x);
        }
    }
    
    Set(bits_t bits): bits(bits) {
        for(node_t node = 0; node < bits.size(); node++)
            if(bits.test(node))
                vector.push_back(node);
    }

    void clear() {
        bits.reset();
        vector.clear();
    }
    
    friend std::ostream& operator<<(std::ostream&, const Set&);
    
private:
    std::vector<node_t> vector;
    bits_t bits;
};

inline std::ostream& operator<<(std::ostream& os, const Set& set) {
    os << "[";
    for(node_t i = 0; i < set.size(); i++) {
        if(i > 0) os << ",";
        os << set[i];
    }
    return os << "]";
}

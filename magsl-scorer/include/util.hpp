#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <bitset>
#include <map>
#include <sstream>

using bits_t = std::bitset<32>;

template<typename T>
inline void print(const T& item) {
    std::cout << item << std::flush;
}

template<typename T, typename... L>
void print(const char* fstr, const T& item, L... rest) {
    for(; *fstr; fstr++)
        if(*fstr == '\\') {
            fstr++;
            std::cout << *fstr << std::flush;
        }
        else if(*fstr == '%') {
            std::cout << item << std::flush;
            print(fstr + 1, rest...);
            break;
        }
        else
            std::cout << *fstr << std::flush;
    std::cout << std::flush;
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

template<typename T>
inline std::string to_string(const T& value) {
    std::stringstream stream;
    stream << value;
    return stream.str();
}

inline size_t binomial(size_t n, size_t k) {
    static std::map<std::pair<size_t, size_t>, size_t> cache;
    if(k == n || k == 0)
        return 1;
    auto it = cache.find( std::make_pair(n, k) );
    if(it == cache.end())
        it = cache.insert(cache.begin(), std::make_pair(
            std::make_pair(n, k), binomial(n - 1, k - 1) + binomial(n - 1, k)
        ));
    return it->second;
}

template<typename T>
std::string to_string(const std::vector<T>& vec) {
    std::string s;
    for(size_t i = 0; i < vec.size(); i++) {
        if(i > 0) s += ",";
        s += to_string(vec[i]);
    }
    return s;
}

#define ensure(expr, ...) \
    if(!(expr)) { print("Not satisfied: %\n", #expr); std::exit(0); }

inline size_t get_time() {
    auto x = std::chrono::system_clock::now().time_since_epoch();
    auto y = std::chrono::duration_cast< std::chrono::microseconds >(x);
    return y.count();
}

#define percentage(a, b) ((b) == 0? 0: int(double(a)/double(b)*10000) / double(100) )
#define in_seconds(t) (double((t)/1000)/1000)
#define duration_since(t) in_seconds(get_time() - (t))

inline std::string capitalize(std::string word) {
    if(word.length() == 0) return word;
    if(word[0] >= 'a' && word[0] <= 'z')
        word[0] += 'A' - 'a';
    return word;
}

#define BENCHMARK(title, ...) \
    { print("%", capitalize(title) + (std::string)"...\n");\
      size_t tic = get_time();\
      __VA_ARGS__;\
      print("Done %, took %s\n", title, duration_since(tic)); }

#define error(...) { print("Error: " __VA_ARGS__); std::exit(-1); }

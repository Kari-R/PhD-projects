#pragma once

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <limits>

#define DEBUG 1

inline size_t get_time() {
    auto x = std::chrono::system_clock::now().time_since_epoch();
    auto y = std::chrono::duration_cast< std::chrono::microseconds >(x);
    return y.count();
}

template<typename Arg>
std::string format(const Arg& arg) {
    std::stringstream stream;
    stream << std::setprecision(14);
    stream << arg;
    return stream.str();
}

template<typename Arg, typename... Args>
std::string format(const char * f, const Arg& arg, Args... args) {
    std::stringstream stream;
    for(; *f; f++) {
        if(*f == '\\') {
            f++;
            stream << *f;
            continue;
        }
        if(*f == '%') {
            stream << std::setprecision(14);
            stream << arg << format(f + 1, args...);
            break;
        }
        stream << *f;
    }
    return stream.str();
}

template<typename T>
std::string to_string(const T& t) {
    std::stringstream stream;
    stream << std::setprecision(14);
    stream << t;
    return stream.str();
}

template<bool Left = true, typename T>
std::string justify(T item, size_t len) {
    std::string s = to_string(item);
    while(s.length() < len)
        s = Left? s + " ": " " + s;
    return s;
}

template<typename T>
T to_number(const std::string& s) {
    std::stringstream stream(s);
    T number;
    stream >> number;
    return number;
}

inline std::string format_milliseconds(size_t ms) {
    std::string str = to_string(ms);
    if(str.length() >= 4)
        return str.substr(0, str.length() - 3) + "." +
            str.substr(str.length() - 3, str.length()) + "s";
    while(str.length() < 3)
        str = "0" + str;
    return "0." + str + "s";
}

template<typename... Args>
void print(Args... args) {
    std::cout << format(args...) << std::flush;
}

#define BENCHMARK(name, ...) {\
    size_t time_ = get_time();\
    print("%... ", name);\
    __VA_ARGS__;\
    print("Took %\n", format_milliseconds(get_time() - time_));\
}

#if DEBUG
    #define ensure(x) \
        if(!(x)) {\
            print("Assert failed: '%'\n  At %\n", #x, __PRETTY_FUNCTION__);\
            std::exit(0);\
        }
#else
    #define ensure(x) {}
#endif

class Rand {
public:
    using num_t = unsigned long long;
    
    static unsigned long long& seed() {
        static num_t value = 1;
        return value;
    }
    
    static num_t get() {
        return seed() = (seed() * (num_t)16807) % (num_t)(2147483647);
    }
    
    static size_t get(size_t a, size_t b) {
        ensure(a != b);
        return b < a? get(b, a):
            a + get() % (b - a);
    }
    
    static void distinct_pair(size_t ub, size_t& a, size_t& b) {
        ensure(ub >= 2);
        a = get() % ub;
        do b = get() % ub; while(a == b);
    }
};

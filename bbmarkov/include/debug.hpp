#pragma once

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

#define DEBUG 0

typedef unsigned int uint;

inline size_t get_time() {
    auto x = std::chrono::system_clock::now().time_since_epoch();
    auto y = std::chrono::duration_cast< std::chrono::milliseconds >(x);
    return y.count();
}

template<typename Arg>
std::string format(const Arg& arg) {
    std::stringstream stream;
    stream << arg;
    return stream.str();
}

template<typename Arg, typename... Args>
std::string format(const char * f, const Arg& arg, Args... args) {
    std::stringstream stream;
    for(; *f; f++) {
        if(*f == '%') {
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
    stream << std::setprecision(15);
    stream << t;
    return stream.str();
}

struct Output {
    int indent_lvl;
    std::ostream& target;
    
    Output(std::ostream& os):
        target(os),
        indent_lvl(0) {}
    
    template<typename... Args>
    void operator()(Args... args) {
        for(uint i = 0; i < indent_lvl; i++)
            target << "  ";
        target << format(args...) << std::flush;
    }
    
    struct IndentGuard {
        Output& o;
        IndentGuard(Output& o): o(o) { o.indent_lvl++; }
        ~IndentGuard() { o.indent_lvl--; }
    };

    static Output& standard() {
        static Output output(std::cout);
        return output;
    }
};

template<typename... Args>
void print(Args... args) {
    Output::standard()(args...);
}

template<typename T = long double, typename I>
T read(I& input) {
    T value;
    input >> value;
    return value;
}

#if DEBUG
    #define open_indent_scope() \
        volatile Output::IndentGuard guard(Output::standard());

    #define assert(x) \
        if(!(x)) {\
            print("Assert failed: '%'\n\tAt %\n", #x, __PRETTY_FUNCTION__);\
            std::exit(0);\
        }

    #define verbose(...) \
        if(flags[MaxVerbosity]) { print(__VA_ARGS__); }
#else
    #define open_indent_scope() /**/
    #define assert(x) /**/
    #define verbose(...) /**/
#endif

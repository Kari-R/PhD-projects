#pragma once

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <limits>

#define DEBUG 0

#define IF_CPLEX(...) if(!instance[DisableCPLEX]) { __VA_ARGS__ }

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
        if(*f == '\\') {
            f++;
            stream << *f;
            continue;
        }
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

template<typename... Args>
void header(Args... args) {
    for(size_t i = 0; i < 30; i++)
        print("=");
    print(" ");
    print(args...);
}

#define ADD_SPACING \
    for(size_t _d = 0; _d < depth * 2; _d++)\
        print(".");

#if DEBUG
    #define ensure(x) \
        if(!(x)) {\
            print("Assert failed: '%'\n  At %\n", #x, __PRETTY_FUNCTION__);\
            std::exit(0);\
        }

    #define ideal_eq(a, o, b, ...) \
        if(!((a) o (b))) {\
            print("Ideal condition not satisfied: % % %\n  At %\n", #a, #o, #b, __PRETTY_FUNCTION__);\
            print("  with % = '%', % = '%'.\n", #a, (a), #b, (b));\
            __VA_ARGS__\
            std::cin.get();\
        }

    #define ensure_eq(a, o, b) \
        if(!((a) o (b))) {\
            print("Assert failed: % % %\n  At %\n", #a, #o, #b, __PRETTY_FUNCTION__);\
            print("Where\n  %: '%'\n  %: '%'\n", #a, (a), #b, (b));\
            std::exit(0);\
        }
#else
    #define ensure(x) {}
    #define ensure_eq(a, o, b) {}
    #define ideally_eq(a, o, b) {}
#endif

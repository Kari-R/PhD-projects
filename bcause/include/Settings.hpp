#pragma once

#include <debug.hpp>
#include <vector>

#include "Instance.hpp"

typedef unsigned char uchar;
typedef long Weight;

#define INF std::numeric_limits<Weight>::max()

#define print_error(...) \
    { print("\nError: " __VA_ARGS__); std::exit(0); }

enum Argument {
    NotLimited = -1,
    Heuristic,
    MaxVars,
    MaxCond,
    MaxDegree,
    ThreadCount,
    MaxBidirCount,
    Acyclicity,
    DisableCPLEX,
    CorePropagation,
    RCFixing,
    Verbose,
    SymmetryBreaking1,
    SymmetryBreaking2,
    DisableRelevancyRules,
    RelevancyAlgorithm,
    SigmaSeparation,
    NoncoreConstraints,
    ArgCount,
};

enum Heuristics {
    IndepsFirst,
    DependsFirst,
    Hybrid,
    Random,
    InvalidHeuristic,
};

class Settings {
public:
    static constexpr const char* Symbols = "hncdtbaCkrv12RAsN";
    typedef long ArgT;
    
    inline const std::string& path() const { return fname; }
    
    inline ArgT operator[](Argument n) const { return values[n]; }
    
    Settings(): values(32, NotLimited), starting_weight(INF) {
        for(size_t bool_arg = Acyclicity; bool_arg < ArgCount; bool_arg++)
            values[bool_arg] = false;
        values[Heuristic]       = (ArgT)Heuristics::IndepsFirst;
        values[ThreadCount]     = 1;
    }
    
    void parse(int argc, const char** argv) {
        N = argc;
        arguments = argv;
        for(i = 1; i < N;)
            if(argv[i][0] == '-')
                parse_arg(arguments[i]);
            else {
                if(fname != "")
                    print_error("Filename given multiple times.\n");
                fname = argv[i];
                i++;
            }

        if(values[Heuristic] < 0 || values[Heuristic] >= (ArgT)InvalidHeuristic)
            print_error("Invalid heuristic (-%). Should be 0-%.\n",
                Symbols[Heuristic], (ArgT)InvalidHeuristic - 1);
        
        if((values[CorePropagation] || values[RCFixing]) && values[DisableCPLEX])
            print_error("RC fixing (-r) requires CPLEX (-C).\n");
    }

    inline const std::string& fixed_edges() const {
        return fixing_string;
    }
    
    inline Weight starting_ub() const {
        return starting_weight;
    }
    
protected:
    void parse_arg(const std::string& argument) {
        i++;
        for(size_t j = 1; j < argument.length(); j++) {
            if(argument[j] == 'f') {
                if(fixing_string != "")
                    print_error("Edge fixing argument given multiple times.\n");
                fixing_string = arguments[i];
                i++;
                continue;
            } else if(argument[j] == 'w') {
                if(starting_weight != INF)
                    print_error("Starting weight given multiple times.\n");
                std::stringstream(arguments[i]) >> starting_weight;
                if(starting_weight == 0 && std::string(arguments[i]) != "0")
                    print_error("Invalid starting weight (-s flag)\n");
                i++;
                continue;
            }
            int arg = std::string(Symbols).find(argument[j]);
            if(arg == -1)
                print_error("Unrecognized argument '%'\n", argument[j]);
            if(arg >= Acyclicity) {
                values[arg] = true;
                continue;
            }
            if(i == N || arguments[i][0] == '-')
                print_error("Argument '%' requires a value.\n", argument[j]);

            std::stringstream stream(arguments[i]);
            size_t val;
            stream >> val;
            if(val == 0 && arguments[i][0] != '0')
                print_error("Invalid value for argument '%'.\n", argument[j]);
            values[arg] = (ArgT)val;
            i++;
        }
    }

private:
    size_t i, N;
    const char** arguments;
    std::vector<ArgT> values;
    std::string fname, fixing_string;
    Weight starting_weight;
};

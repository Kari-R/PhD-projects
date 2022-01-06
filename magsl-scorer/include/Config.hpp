#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Config {
public:    
    struct Arg {
        enum value_t {
            String,
            Boolean,
            Integer,
            Float,
        };
        
        Arg(const std::string&, const std::string&, const std::string&);
        
        Arg& flip();
        Arg& operator=(const std::string&);
        value_t deduce_type(const std::string&);

        std::string name, strval, descr;
        double numval;
        value_t type;
    };
    
    void read(size_t, char**);
    void read(const std::string&);
    void read(const std::vector<std::string>&);
    
    void add(const std::string&, const std::string&, const std::string&);
    void put_spaces(size_t);
    void output();
    
    void set(const std::string&, const std::string&);
    
    static Config& default_config() {
        static Config config;
        return config;
    }
    
    inline size_t count() const { return named_args.size(); }
    
    inline double get(size_t i) const { return named_args[i].numval; }
    inline bool has(size_t i) const { return named_args[i].numval; }
    inline const std::string& string(size_t i) const { return named_args[i].strval; }
    
    inline const std::vector<std::string>& unnamed_arguments() const { return unnamed_args; }
    
private:
    std::vector<Arg> named_args;
    std::vector<std::string> unnamed_args;
};

#define DEFINE_FLAG(const_name, name, defval, descr) \
    static constexpr size_t const_name = __COUNTER__;\
    struct Tmp_##const_name { Tmp_##const_name(){\
        if(const_name == Config::default_config().count())\
            Config::default_config().add(name, #defval, descr);\
        }};\
    static Tmp_##const_name tmp_##const_name;

DEFINE_FLAG(MaxVars, "max-vars", -1,
    "Only consider N variables.");

DEFINE_FLAG(MaxParsPerNode, "max-pars", -1,
    "Limits the number of parents a variable can have.")

DEFINE_FLAG(MaxComp, "max-comp", -1,
    "Limits the maximum c-component size.")

DEFINE_FLAG(ICF_Tolerance, "icf-tol", 0.000001,
    "The minimum allowed progress in ICF until convergence.")

DEFINE_FLAG(Max_ICF_Iters, "max-iters", -1,
    "Limits the maximum number of ICF iterations (per score).")

DEFINE_FLAG(PruneLevel, "prune", 1,
    "Sets the prune level. 0=no pruning, 1=some pruning, 2=full pruning")

DEFINE_FLAG(InputCovariances, "no-data", false,
    "The input file contains the sample count and covariances as opposed to raw data.")

DEFINE_FLAG(MaxParsPerComp, "max-pars-per-comp", -1,
    "Limits the maximum number of parents per component.")

DEFINE_FLAG(GraphDir, "graph",,
    "Scores the graph from the given file.")

DEFINE_FLAG(ICF_Repetitions, "repeat", 1,
    "If >1, runs ICF multiple times from random starting points.")

DEFINE_FLAG(ICF_Max_Optimas, "max-optimas", 1000,
    "The \"maximum\" number of local optimas for ICF to find.")

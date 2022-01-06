#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Config {
public:
    struct Arg {
        enum value_t {
            STRING,
            BOOLEAN,
            INTEGER,
            FLOAT,
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
    struct Dummy_##const_name { Dummy_##const_name(){\
        if(const_name == Config::default_config().count())\
            Config::default_config().add(name, #defval, descr);\
        }};\
    static Dummy_##const_name dummy_##const_name;

DEFINE_FLAG(SIMPLIFY, "simplify", false,
    "Apply further simplifications to the reachability matrices.")

DEFINE_FLAG(CHECK_LB_MAXIMALITY, "check-lbs", false,
    "Check if UB solutions are maximal and backtrack whenever so.")

DEFINE_FLAG(EXPERIMENTAL, "experimental-features", false,
    "Use experimental (but not throughoutly tested) optimizations to speed up the search.")

#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <util.hpp>

class Config {
public:
    using NumType = double;
    
    static Config& default_config() {
        static Config config;
        return config;
    }
    
    struct Arg {
        std::vector<std::string> names;
        std::string str, description;
        NumType number;
        bool is_bool;
        
        inline bool contains(char c) const { return str.find(c) != std::string::npos; }
        inline bool on() const { return number != 0; }
        
        inline bool operator==(NumType val) const { return number == val; }
        inline bool operator<=(NumType val) const { return number <= val; }
        inline bool operator>=(NumType val) const { return number >= val; }
        inline bool operator<(NumType val)  const { return number < val; }
        inline bool operator>(NumType val)  const { return number > val; }
        inline bool operator!=(NumType val) const { return number != val; }
        
        inline bool operator!() const { return !number; }
        
        inline NumType value() const { return number; }
    };

    void add(const std::vector<std::string>& cmd, const std::string& description, const std::string& value) {
        for(const std::string& name: cmd)
            names.insert(std::make_pair(name, args.size()));
        args.push_back({cmd, value, description,
            to_number<NumType>(value), value == "true" || value == "false"});
    }
    
    void set(size_t arg, const char* value) {
        args[arg].str = value;
        args[arg].number = to_number<NumType>(args[arg].str);
    }

    inline Arg operator[](size_t arg) const {
        ensure(arg < args.size());
        return args[arg];
    }
    
    const std::string& as_string() const { return string; }
    
    void read(int, char**);
    void output() const;
    
    inline size_t count() const { return args.size(); }
    inline const std::string& filename() const { return input_fname; }

    std::string name() const {
        auto start = filename().rfind('/');
        auto end = filename().rfind('.');
        if(start == std::string::npos) start = 0; else start++;
        if(end == std::string::npos) end = filename().length();
        return filename().substr(start, end - start);
    }
    
private:
    std::string input_fname, string;
    std::vector<Arg> args;
    std::unordered_map<std::string, size_t> names;
};

#define DEFINE(name, args, default_value, description) \
    static constexpr size_t name = __COUNTER__;\
    struct Dummy_##name { Dummy_##name(){\
        if(name == Config::default_config().count())\
            Config::default_config().add(args, description, #default_value);\
        }};\
    static Dummy_##name dummy_##name;

#define LIST(...) std::vector<std::string>{__VA_ARGS__}

DEFINE(GREEDY_BASELINE, LIST("--greedy"), false,
    "Use greedy search instead of a stochastic one." )

DEFINE(RESTART_RATE, LIST("--restart-rate"), 1000,
    "Maximum number of iterations without local improvement.")

DEFINE(ESS, LIST("--ess"), 1,
    "Sets the equivalent sample size when computing scores.")

DEFINE(MAX_CLIQUE, LIST("--max-clique"), -1,
    "Limits the maximum clique size (for CSV inputs). -1=limitless")

DEFINE(PRECOMP_LVL, LIST("--precompute"), 0,
    "Precompute clique scores (for CSV inputs) 0=no, 1=some, 2=all")
        
DEFINE(DYNAMIC_TW, LIST("--dynamictw"), false,
    "Start with small clique limit and then increase (3, 4, 8, 16, ...)")

DEFINE(FIXED_ITERS, LIST("--fixed-iters"), false,
    "Have a fixed number of iterations between restarts.")

DEFINE(USE_CHOW_LIU, LIST("--chow-liu"), false,
    "Use Chow-Liu to find the optimal tree in the beginning.")
        
DEFINE(OPERATIONS, LIST("--operations"), "CEV",
    "Operations to use; [C]lique, [E]dge, [V]ertex; e.g. CE")

DEFINE(RNG_SEED, LIST("--seed"), 0,
    "Set a custom RNG seed. 0=random")
        
#include <util.hpp>
#include <Instance.hpp>
#include <vector>
#include <unordered_set>
#include <set>
#include <util.hpp>
#include <algorithm>
#include <chrono>

#include <Search.hpp>

Config get_config(int, char**);

int main(int argc, char** argv) {    
    size_t start_time = get_time();
    Config config = get_config(argc, argv);

    std::time_t date = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    
    print(std::ctime(&date));
    print("Arguments: ");
    for(size_t i = 0; i < (size_t)argc; i++)
        print("% ", argv[i]);
    print("\n\n");
    Instance instance(config);

    Rand::seed() = !config[RNG_SEED].value()?
        get_time()/1000: config[RNG_SEED].value();
    print("Using RNG seed %\n", Rand::seed());
    
    Search search(instance, start_time);
    search.search();
}

Config get_config(int argc, char** argv) {
    Config config = Config::default_config();
    config.read(argc, argv);
    
    if(config.filename() == "") {
        print("Usage: % <dataset name> [arguments...]\n", argv[0], argv[0]);
        print("\nArguments (defaults in parentheses):\n");
        config.output();
        std::exit(-1);
    }
    
    long tw = config[MAX_CLIQUE].value();
    if(tw != -1 && tw < 2) {
        print("Error: Clique size limit cannot be smaller than 2.\n");
        std::exit(-1);
    }
    
    for(char c: config[OPERATIONS].str)
        if(c != '\"' && c != 'C' && c != 'E' && c != 'V') {
            print("Error: Operations need to be either E, C, V, CE, VE, CV or CVE.\n");
            std::exit(-1);
        }
    if(config[OPERATIONS].str.length() == 0) {
        print("Error: Invalid operations given as argument.\n");
        std::exit(-1);
    }
    
    long precomp = config[PRECOMP_LVL].value();
    if(precomp < 0 || precomp > 2 || precomp != (int)precomp) {
        print("Error: Precomputation level needs to be either 0, 1 or 2.\n");
        std::exit(-1);
    }
    return config;
}

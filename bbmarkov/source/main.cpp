#include <Solver.hpp>
#include <iomanip>
#include <debug.hpp>
#include <cstdlib>

std::bitset<32> parse_flags(int argc, char ** argv) {
    std::bitset<32> flags;
    for(size_t i = 0; i < argc; i++) {
        if(argv[i][0] != '-')
            continue;
        char * orig = argv[i];
        for(argv[i]++; * argv[i]; argv[i]++) {
            if(flag_symbols.find(* argv[i]) == -1) {
                print("Unrecognized flag '-%'.\n", * argv[i]);
                std::exit(-1);
            }
            flags[flag_symbols.find(* argv[i])] = true;
        }
        argv[i] = orig;
    }
    return flags;
}

int main(int argc, char ** argv) {
    
    std::string fname;
    for(size_t i = 1; i < argc; i++)
        if(argv[i][0] && argv[i][0] != '-')
            fname = argv[i];
    
    auto flags = parse_flags(argc, argv);
    if(fname != "") {        
        if(!flags[MinVerbosity])
            print("Reading the input file... ");

        Instance instance(fname);
        if(!instance.is_valid()) {
            print("Couldn't open '%' or its contents are invalid.\n", fname);
            std::exit(-1);
        }

        if(!flags[MinVerbosity])
            print("Done.\n");
        
        Solver solver(flags, instance, instance.domains.size());
        solver.solve();
    } else
        print("Usage: % [flags] <file>\n\n"
            "Flags:\n"
            "  -v    Minimal verbosity\n"
            "  -s    Keep track of the number of recurring skeletons (for debugging purposes)\n"
            "  -o    Prune recurring option lists\n"
            "  -t    Use tight upper bounds that take immoralities into consideration\n"
            "  -f    Fix an arbitrary variable to be the first in the ordering\n\n"
            "Recommended flags are -ft. (-o is good with some instances)\n", argv[0]);
}
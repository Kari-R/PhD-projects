#include <BranchAndBound.hpp>
#include <Settings.hpp>
#include <cstdlib>

int main(int argc, const char** argv) {
    if(argc < 2) {
        const char* info =
            "Version 2.0 (PGM 2020)\n"
            "Usage: % [<flags>] <path to input file>\n\n"
            "Flags for defining the search space:\n"
            "  -n <number>     Limit the number of nodes/variables.\n"
            "  -c <number>     Limit maximum conditioning set size.\n"
            "  -d <number>     Limit maximum node degree.\n"
            "  -b <number>     Limit maximum number of <-> edges.\n"
            "  -a              Enforce acyclicity\n"
            "  -s              Use sigma-separation instead of d-separation\n"
            "  -w <number>     Only search for graphs that have lower weight than this.\n"
            "  -f <string>     Force solutions to contain certain edges. For example:\n"
            "                           -f \"1<>2 2->3 3<-5\"\n"
            "                      or   -f edges.txt\n"
            "                      (\"1</>2 2/>3 3</5\" enforces edge inexistence.)\n\n"
            "Flags for tuning the search:\n"
            "  -h <number>     Heuristic for selecting next node pair (X,Y) (default: 0)\n"
            "                    0. Select pair most likely independent (edges absent)\n"
            "                    1. Select pair most likely dependent (edge(s) present)\n"
            "                    2. A hybrid of 0 and 1.\n"
            "                    3. Branch randomly.\n"
            "  -N              Enable dataset linking rules for CPLEX\n"
            "  -R              Disable edge relevancy rules (not recommended)\n"
            "  -C              Disable CPLEX (not recommended)\n"
            "  -A              Apply further edge relevancy checks via separate algorithm\n"
            "  -t <number>     Set the number of threads CPLEX can use (default: 1)\n"
            "  -v              Enable verbosity (only for debugging purposes)\n";
        print(info, argv[0]);
        std::exit(0);
    }

#if DEBUG
    print("\n  * Warning: Debugging (#DEBUG in debug.hpp) is on:\n"
        "  * Will affect performance.\n\n");
#endif
    
    srand(0);
    Settings settings;
    settings.parse(argc, argv);
    
    print("Given arguments:");
    for(size_t i = 0; i < (size_t)argc; i++)
        print(" %", argv[i]);
    print("\n");

    if(settings.path() == "")
        print_error("Need a path to input file.\n");
    
    std::ifstream file(settings.path());
    if(!file.is_open())
        print_error("Unable to open '%'\n", settings.path());
    
    Instance instance(settings, file);
    print("N=% with % independences, % dependences, % interventions.\n",
        (int)instance.size(),
        instance.independent.size(),
        instance.dependent.size(),
        instance.by_intervent.size() - 1);

    BranchAndBound bb(instance);
    bb.solve();
    bb.print_stats();
}

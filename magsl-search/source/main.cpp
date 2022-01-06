#include <util.hpp>
#include <bitset>
#include <vector>

#include <Config.hpp>
#include <DPSolver.hpp>
#include <BBSolver.hpp>

#include <functional>
#include <algorithm>

int main(int argc, char** argv) {
    size_t start_time = get_time();
    Config config = Config::default_config();
    
    config.read(argc, argv);
    if(config.unnamed_arguments().size() != 1) {
        print("Usage: % <path> [flags]\n\n", argv[0]);
        config.output();
        std::exit(0);
    }
    
    std::string path = config.unnamed_arguments()[0];

    std::ifstream file(path);
    ensure(file.is_open());
    
    Scores scores(file);
    scores.preprocess(config);

    print("\n========================= BN Search\n");
    print("Starting by finding the optimal BN structure...\n"
        "If this takes a long time, then the optimal MAG will be hard to find.\n\n");

    size_t bn_start = get_time();
    DPSolver dp(config, scores);
    const Solution& bn = dp.learn_BN();

    print("Optimal BN structure (%s):\n", duration_since(bn_start));
    bn.output("BN");

    print("\n========================= MAG Search\n");
    print("Now searching for the optimal MAG structure...\n");
    BBSolver bb(config, scores);
    bb.solve(start_time, config.has(EXPERIMENTAL)? &bn: nullptr);
    print("Total time: %s\n", duration_since(start_time));
}

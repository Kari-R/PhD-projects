#include <ICF.hpp>
#include <ScoreSearch.hpp>
#include <Scorer.hpp>
#include <Config.hpp>
#include <fstream>
#include <algorithm>
#include <iomanip>

template<typename Fun>
void for_each_subset(size_t max_var, size_t count, const Fun& fun, bits_t subset = 0, size_t start = 0) {
    if(subset.count() == count)
        fun(subset);
    else for(size_t var = start; var < max_var; var++)
        for_each_subset(max_var, count, fun, bits_t(subset).set(var), var + 1);
}

auto extract_path(const std::string& path) {
    std::string dir, basename, ext;
    if(path.rfind('/') != path.npos) {
        dir = path.substr(0, path.rfind('/') + 1);
        basename = path.substr(path.rfind('/') + 1, path.length());
    } else
        basename = path;
    if(basename.rfind('.') != basename.npos) {
        ext = basename.substr(basename.rfind('.') + 1, basename.length());
        basename = basename.substr(0, basename.rfind('.'));
    }
    return std::make_tuple(dir, basename, ext);
}

int main(int argc, char** argv) {
    size_t start_time = get_time();
    
    Config config = Config::default_config();
    config.read(argc, argv);
    
    if(config.unnamed_arguments().size() != 1) {
        print("Usage: % <input-file> [flags]\n\n", argv[0]);
        config.output();
        return 0;
    }
    
    std::string input_path = config.unnamed_arguments()[0];
    auto path = extract_path(input_path);
    std::string 
        directory = std::get<0>(path),
        basename = std::get<1>(path);
    
    std::ifstream input_file(input_path);
    if(!input_file.is_open())
        error("Unable to open '%'.\n", input_path);
    
    size_t max_vars = config.get(MaxVars) != -1?
        (size_t)config.get(MaxVars): (size_t)Scoreable::Unlimited;
    
    Graph* graph = nullptr;
    if(config.string(GraphDir) != "") {
        std::string path = config.string(GraphDir);
        std::ifstream file(path);
        if(!file.is_open())
            error("Unable to open the graph file.\n");
        if(path.find(".ccd") != std::string::npos)
            graph = new Graph(Graph::from_ccd_output(file));
        else if(path.find(".magbb") != std::string::npos)
            graph = new Graph(Graph::from_own_output(file));
        else
            graph = new Graph(Graph::from_edge_matrix(file));
        max_vars = graph->size();
    }

    ICF icf(config.get(ICF_Tolerance));
    if(config.has(InputCovariances))
        icf.read_covariances(input_file, max_vars);
    else
        icf.read_data(input_file, max_vars);
    input_file.close();
    
    if(graph != nullptr) {
        if(graph->size() != icf.variable_count())
            error("Wrong number of variables in the graph file.\n");
        
        double score = graph->score(icf, config.get(ICF_Repetitions), config.get(ICF_Max_Optimas));
        std::cout << std::setprecision(10) << score << std::endl;
        return 0;
    }

#define from_config(flag) size_t(config.get(flag) != -1? config.get(flag): 1000000)
    
    size_t variable_count = icf.variable_count();
    size_t max_component = std::min(variable_count, from_config(MaxComp));
    size_t max_pars_per_node = std::min(variable_count - 1, from_config(MaxParsPerNode));
    size_t max_pars_per_comp = std::min(max_pars_per_node * max_component, from_config(MaxParsPerComp));
    
    max_pars_per_node = std::min(max_pars_per_node, max_pars_per_comp);
    
    if(variable_count > bits_t().size())
        error("The number of variables can be at most 32.\n");

    if(max_component > 5)
        print("\n** Warning: Maximum component size is %. Likely impossible to score. **\n\n",
            max_component);
    
    std::string out_name = basename;
    if(config.get(MaxVars) != -1)
        out_name += "-n" + to_string(variable_count);

    if(config.get(Max_ICF_Iters) != -1)
        icf.MaxIters = config.get(Max_ICF_Iters);

    print("Vars: %, Max-Pars-Per-Node: %, Max-Comp: %, Max-Pars-Per-Comp: %\n",
        variable_count, max_pars_per_node, max_component, max_pars_per_comp);
    
    if(config.get(MaxComp) != -1) out_name += "-c" + to_string(max_component);
    if(config.get(MaxParsPerNode) != -1) out_name += "-p" + to_string(config.get(MaxParsPerNode));
    if(config.get(MaxParsPerComp) != -1) out_name += "-q" + to_string(config.get(MaxParsPerComp));
    
    if(config.get(ICF_Repetitions) > 1) {
        out_name += "-r" + to_string(config.get(ICF_Repetitions));
        if(config.get(ICF_Max_Optimas) != 1000)
            out_name += "-o" + to_string(config.get(ICF_Max_Optimas));
    }

    std::ofstream output_file(directory + out_name + ".bic");
    output_file << variable_count << " " << max_pars_per_node << " " << max_component << std::endl;

    Scorer scorer(config, &icf, max_pars_per_node, max_pars_per_comp, output_file);
    size_t total_count = config.get(MaxParsPerComp) != -1? 0: scorer.output_total_score_count(max_component);
    
    for(size_t comp_size = 1; comp_size <= max_component; comp_size++) {
        print("======================= %-node c-components\n", comp_size);
        
        Scoreables scoreables = ScoreSearch(comp_size, max_pars_per_node).search();
        size_t iters = 1;
        for_each_subset(icf.variable_count(), comp_size, [&](bits_t labeling) {
            std::vector<node_t> component;
            for(node_t i = 0; i < labeling.size(); i++)
                if(labeling.test(i))
                    component.push_back(i);
            print("[%s, %%] Next c-component: {%}\n", 
                duration_since(start_time),
                config.get(MaxParsPerComp) != -1? "unknown": to_string(percentage(icf.amount_scored(), total_count)),
                '%', to_string(component));
            print("  Calculating scores...");
            for(const Scoreable& scoreable: scoreables)
                scorer.iterate_extensions(scoreable, labeling);
            print(" Done.\n");
            scorer.prune_and_output();
            iters++;
        });
    }
    
    print("======================= Finished\n");
    print("Ran ICF % times in %s. (% runs/s)\n",
        icf.amount_scored(), duration_since(start_time),
        double(icf.amount_scored()) / duration_since(start_time));
    print("ICF required %s. (%% of total time)\n", in_seconds(icf.time_spent()),
        percentage(icf.time_spent(), get_time() - start_time), '%');
    print("Average ICF iters: %    Most ICF iters: %\n",
        size_t(double(icf.avg_iterations()) / config.get(ICF_Repetitions) + 0.5), icf.max_iterations());
    if(config.get(PruneLevel) > 0)
        print("Pruning took %s.\n", in_seconds(scorer.total_prune_time()));
    print("Scores-Before-Pruning: %\n", scorer.total_score_count());
    print("Scores-After-Pruning: %\n", scorer.total_score_count() - scorer.pruned_score_count());
    if(config.get(ICF_Repetitions) > 1) {
        auto before = scorer.optima_rate_before_pruning();
        print("Multiple-Optimas-Before-Pruning: %\n", before.first);
        print("Most-Optimas-Before-Pruning: %\n", before.second);
        
        auto after = scorer.optima_rate_after_pruning();
        print("Multiple-Optimas-After-Pruning: %\n", after.first);
        print("Most-Optimas-After-Pruning: %\n", after.second);
    }
    print("\nScores were printed into:\n%\n",
        directory + out_name + ".bic");
}

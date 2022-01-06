#include <Solver.hpp>
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <debug.hpp>

#define max(a, b) ((a) > (b)? (a): (b))
#define min(a, b) ((a) < (b)? (a): (b))

Solver::Solver(const std::bitset<32>& f, Instance& i, uint mv):
    flags(f),
    state(0, i.domains.size(), mv),
    lb_solution(-INF, i.domains.size(), mv),
    bounds(state, i, mv),
    stats {std::vector<size_t>(mv, 0), get_time(), 0, 0, 0, 0, -INF, INF},
    instance(i),
    N(mv) {}

const Solution& Solver::solution() const {
    return lb_solution;
}

std::string formatted_time(size_t time) {
    std::string s;
    if(time >= 1000 * 60) {
        s += to_string(time / (1000 * 60)) + "m ";
        time %= 1000 * 60;
    }
    if(time >= 1000) {
        s += to_string(time / 1000) + "s ";
        time %= 1000;
    }
    return s + to_string(time) + "ms";
}

void Solver::output_lowerbound(const std::string& title) {
    if(flags[MinVerbosity]) {
        std::cout << stats.time() << "\t"
            << to_string(lb_solution.score) << "\t"
            << title << std::endl;
        if(title != "Solution")
            return;
    }
    std::cout << "-------- " << title << " -------- [" <<
        formatted_time(stats.time()) << "]\n";
    lb_solution.print(instance.domains);
    std::cout << std::endl;
}

Score Solver::generate_lowerbound() {    
    if(!flags[MinVerbosity]) {
        Network network = bounds.upperbound_solution();

        std::cout << "-------- Optimal Bayes -------- ["
            << formatted_time(stats.time()) << "]\n";
        Solution::construct(instance.domains, instance.domain_lookup, network).print(instance.domains);
        std::cout << std::endl;
    }
    
    if(lb_solution.score != -INF)
        output_lowerbound("Initial LB");

    Network network = bounds.lowerbound_solution(10);
    if(network.score > lb_solution.score) {
        lb_solution = Solution::construct(instance.domains, instance.domain_lookup, network);
        output_lowerbound("Moralized Bayes");
    }
    return network.score;
}

void Solver::solve() {
    if(flags[FixedSourceVertex]) {
        state[0].depth = 0;
        state[0].parentset = instance.domain_lookup[0][std::bitset<32>()];

        state.set_parents(0, instance.domains[0][state[0].parentset]);
        state.score = instance.domains[0][state[0].parentset].score;
        verbose("Fixed o_0 <- 0; p_0 <- []\n");
    }
    
    if(!flags[MinVerbosity])
        print("Learning optimal BN structures... ");
    stats.UB = bounds.fast_upperbound();
    if(!flags[MinVerbosity])
        print("Done.\n\n");
    
    stats.LB = generate_lowerbound();
    if(stats.LB < stats.UB)
        solve(state.assign_mask.count());

    output_lowerbound("Solution");
    if(!flags[MinVerbosity])
        output_statistics();
}

Score Solver::choose_last_parentset() {
    uint var = 0;
    while(state.contains(var))
        var++;
    
    if(state.score + instance.domains[var][0].score <= lb_solution.score) {
        verbose("Closing branch: At bottom\n");
        return -INF;
    }

    size_t pset = bounds.best_final_parentset(var);
    if(state.score + instance.domains[var][pset].score <= lb_solution.score) {
        verbose("Closing branch: At bottom\n");
        return -INF;
    }

    state[var].depth = DomainBuilder::get_depth(state, instance.domains[var][pset]);
    state[var].parentset = pset;

    lb_solution = state;
    lb_solution.score += instance.domains[var][pset].score;
    lb_solution.set_parents(var, instance.domains[var][pset]);
    output_lowerbound("New LB");
    return instance.domains[var][pset].score;
}

void Solver::track_skeletons() {
    auto it = skeleton_memo.find(state.skeleton);
    if(it != skeleton_memo.end())
        stats.skeleton_hits++;
    else
        skeleton_memo.insert(std::make_pair(state.skeleton, state));
}

Score Solver::solve(uint order, size_t edges) {
    open_indent_scope();
    stats.layer_visits[order]++;

    if(flags[MemoizeSkeletons])
        track_skeletons();
    
    if(order == N - 1)
        return choose_last_parentset();

    Score original_lb = lb_solution.score;
    Score local_maximum = -INF;

    Score ub = bounds.fast_upperbound();
    if(ub <= lb_solution.score) {
        verbose("Closing branch: UB <= LB\n");
        return -INF;
    }

    if(flags[TightUpperbounds] && N - order < 10 && N - order > 3) {
        float density = float(edges) / float((order * (order - 1)) / 2);
        
        if(density < 0.75) {
            Score ub2 = bounds.tight_upperbound(10, lb_solution.score);

            stats.tight_UBs++;
            if(ub2 <= lb_solution.score) {
                stats.successful_tight_UBs++;
                verbose("Closing branch: Tight UB <= LB\n");
                return ub2;
            }
        }
    }
    
    ValueCalculator value_calc = [&](uint var, size_t pset) {
        state.assign_mask.set(var);
        Score value = instance.domains[var][pset].score + bounds.fast_upperbound();
        state.assign_mask.reset(var);
        return value;
    };

    DomainBuilder options(instance, state,
        flags[SortByUpperbounds] && order <= N / 2?
            &value_calc: nullptr, N);

    for(uint var = 0; var < N; var++)
        if(!state.contains(var))
            options.construct_for(var);

    if(flags[MemoizeOptions] && order <= N - 3) {
        auto match = option_memo.find(options.list());
        if(match != option_memo.end() && match->second + state.score <= lb_solution.score) {
            verbose("Closing branch: The memoized local maximum is low enough.\n");
            stats.option_hits++;
            return match->second;
        }
    }

    verbose("Iterating through % options:\n",
        options.list().size());

    assert(!options.list().empty());

    for(size_t option: options.list()) {

        uint var = option / instance.domains[0].size();
        size_t pset = option % instance.domains[0].size();

        Score original_score = state.score;
        state[var].parentset = pset;
        state[var].depth = DomainBuilder::get_depth(state, instance.domains[var][pset]);
        state.score += instance.domains[var][pset].score;

        state.set_parents(var, instance.domains[var][pset]);
        verbose("Trying p_% <- [%] at position % in the ordering:\n",
            var, instance.domains[var][pset], order);

        Score result = solve(order + 1, edges + instance.domains[var][pset].count());
        if(local_maximum < instance.domains[var][pset].score + result)
            local_maximum = instance.domains[var][pset].score + result;

        state.score = original_score;
        state.reset_parents(var);
        
        if(original_lb != lb_solution.score) {
            if(bounds.fast_upperbound() <= lb_solution.score) {
                verbose("Closing branch: UB <= LB\n");
                break;
            }
            original_lb = lb_solution.score;
        }
    }

    if(flags[MemoizeOptions] && order <= N - 3) {
        if(local_maximum == -INF)
            local_maximum = ub - state.score;
        
        auto pair = option_memo.insert(std::make_pair(options.list(), local_maximum));
        if(!pair.second && pair.first->second > local_maximum) {
            verbose("Improved the memoized local maximum (% -> %).\n",
                pair.first->second, local_maximum);
            pair.first->second = local_maximum;
        }
    }
    return local_maximum;
}

void Solver::output_statistics() {
    print("Search statistics:\n");
    print("  Used % (after the input was read)\n", formatted_time(stats.time()));
    print("  Solution fingerprint: '%'\n", lb_solution.fingerprint());
    
    print("  Gaps:\n");
    print("    Initial UB/Initial LB: % %\n", stats.UB / stats.LB * 100);
    print("    Optimal/Initial LB: % %\n", lb_solution.score / stats.LB * 100);

    size_t total_visits = 0;
    for(uint i = 0; i < N; i++)
        total_visits += stats.layer_visits[i];

    if(skeleton_memo.size() != 0) {
        double skeleton_ratio = double(stats.skeleton_hits) / double(skeleton_memo.size());
        print("  Hits to skeleton cache: %/% (% %)\n",
            stats.skeleton_hits, skeleton_memo.size(), skeleton_ratio * 100);
    }
    if(stats.option_hits != 0) {
        double option_ratio = double(stats.option_hits) / double(option_memo.size());
        print("  Hits to option cache: %/% (% %)\n",
            stats.option_hits, option_memo.size(), option_ratio * 100);
    }
    if(stats.tight_UBs != 0) {
        print("  Tight UB closing a branch: %/% (% %) (after normal UB had failed to do so)\n",
            stats.successful_tight_UBs, stats.tight_UBs,
                double(stats.successful_tight_UBs) / double(stats.tight_UBs) * 100);
    }
    
    auto print_visits = [&](uint i) {
        if(i % 4 == 0)
            print("\n");
        print("    ");
        std::string s = format("%%: % %", i < 10? " ": "", i, (float(stats.layer_visits[i]) / total_visits) * 100);
        while(s.length() < 16)
            s += " ";
        print(s);
    };
    
    double speed = int(double(total_visits) / (double(stats.time()) / 1000));
    print("  Visited % search tree nodes (% nodes/s)\n", total_visits, speed);
    print("  Visit distribution per search tree layer: ");
    for(uint i = 0; i < N; i++)
        print_visits(i);
    print("\n");
}

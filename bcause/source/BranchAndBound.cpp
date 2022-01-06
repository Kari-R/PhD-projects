#include <BranchAndBound.hpp>
#include <algorithm>
#include <cstdlib>

#define N instance.size()

BranchAndBound::BranchAndBound(const Instance& instance):
    Search(instance) {

    if(instance.fixed_edges() != "") {
        apply_edge_fixings();
        update_ub();
    }
    
    find_naive_UB();
    if(ub < instance.starting_ub()) {
        header("Initial UB (%)\n",
            format_milliseconds(get_time() - info.start_time) );
        ub_solution.output();
        print("Weight: %\n", ub);
    } else
        print("Searching, starting with the given initial UB of %...\n", instance.starting_ub());

    gather_initial_information();
    info.time_to_init = get_time() - info.start_time;
}

void extract_edge_info(std::string& edge, size_t& y) {
    size_t center = 0;
    while(edge[center] != 0 && !(edge[center] >= '0' && edge[center] <= '9'))
        center++;
    if(edge[center] == 0) {
        y = INF;
        return;
    }
    std::stringstream stream(edge.substr(center, edge.length()));
    stream >> y;
    edge = edge.substr(0, center);
}

void BranchAndBound::apply_edge_fixings() {
    if(instance.fixed_edges() == "")
        return;
    
    std::string edge_string = instance.fixed_edges();
    if(edge_string.find('.') != std::string::npos) {
        edge_string = "";
        std::ifstream file(instance.fixed_edges());
        if(!file.is_open())
            print_error("Unable to open edge file '%'.\n", instance.fixed_edges());
        while(!file.eof())
            edge_string += file.get();
    }
    
    std::stringstream stream(edge_string);
    std::map<std::string, Edge::Type> map {
        {"<>", Edge::HH_PRESENT}, {"</>", Edge::HH_ABSENT},
        {"<-", Edge::HT_PRESENT}, {"</", Edge::HT_ABSENT},
        {"->", Edge::TH_PRESENT}, {"/>", Edge::TH_ABSENT}
    };
    
    while(!stream.eof()) {
        std::string str;
        size_t x, y;
        stream >> x >> str;
        extract_edge_info(str, y);
        
        if(x < instance.variable_indexing() || y < instance.variable_indexing())
            print_error("The edge fixing string contains too small variables.\n");

        x -= instance.variable_indexing();
        y -= instance.variable_indexing();
        if(x >= (size_t)instance.size() || y >= (size_t)instance.size())
            print_error("The edge fixing string contains too large variables.\n");
        
        if(map.find(str) == map.end())
            print_error("Invalid edge '%' in the edge fixing string.\n", str);

        if(x < y || map[str] == Edge::HH_ABSENT || map[str] == Edge::HH_PRESENT)
            update( Edge(x, y, map[str]) );
        else if(y < x) {
            std::swap(x, y);
            switch(map[str]) {
                case Edge::TH_PRESENT: update(Edge(x, y, Edge::HT_PRESENT)); break;
                case Edge::HT_PRESENT: update(Edge(x, y, Edge::TH_PRESENT)); break;
                case Edge::TH_ABSENT:  update(Edge(x, y, Edge::HT_ABSENT)); break;
                case Edge::HT_ABSENT:  update(Edge(x, y, Edge::TH_ABSENT)); break;
                default: ensure(false);
            }
        }
        
        while(!stream.eof()) {
            if(stream.peek() >= '0' && stream.peek() <= '9')
                break;
            stream.get();
        }
    }
}

void BranchAndBound::gather_initial_information() {
    struct Pair { uchar v1, v2; };
    
    initial_indep.assign(N, std::vector<Weight>(N, 0));
    initial_dep.assign(N, std::vector<Weight>(N, 0));
    
    size_t pair_count = (N * (N - 1)) / 2;
    avg_dep = 0;
    avg_ratio = 0;
    double avg_indep = 0;
    
    std::vector<Pair> by_dep, by_indep;
            
    for(uchar y = 1; y < N; y++)
        for(uchar x = 0; x < y; x++) {
            initial_indep[x][y] = bounds.indep_weight(x, y);
            initial_dep[x][y] = bounds.dep_weight(x, y);
            
            avg_dep += (double)bounds.dep_weight(x, y) / pair_count;
            avg_indep += (double)bounds.indep_weight(x, y) / pair_count;
            avg_ratio += (double)weight_ratio(x, y) / pair_count;
            
            by_dep.push_back(Pair {x, y});
            by_indep.push_back(Pair {x, y});
        }

    std::sort(by_dep.begin(), by_dep.end(), [&](const Pair& p1, const Pair& p2) {
        return bounds.dep_weight(p1.v1, p1.v2) < bounds.dep_weight(p2.v1, p2.v2);
    });
    
    std::sort(by_indep.begin(), by_indep.end(), [&](const Pair& p1, const Pair& p2) {
        return bounds.indep_weight(p1.v1, p1.v2) < bounds.indep_weight(p2.v1, p2.v2);
    });

    median_dep = bounds.dep_weight(by_dep[pair_count / 2].v1, by_dep[pair_count / 2].v2);
}

inline uchar present_edges(const Graph& state, uchar x, uchar y) {
    return (uchar)state.edges(x, y) + state.edges(y, x) +
        state.biedges(x, y);
}

inline uchar decided_edges(const Graph& state, uchar x, uchar y) {
    return (uchar)!state.edge_decided(x, y) +
        !state.edge_decided(y, x) +
        !state.biedge_decided(x, y);
}

inline uchar edge_order(const Edge& decision) {
    return decision.is_TH()? 1:
           decision.is_HT()? 2: 0;
}

bool BranchAndBound::add_edge_first(uchar x, uchar y) const {
    if(present_edges(state, x, y) > 0)
        return false;
    
    if(!initial_dep[x][y])
        return false;
    
    return initial_indep[x][y] == 0 || weight_ratio(x, y) > avg_ratio;
}

bool BranchAndBound::choose_edge(const Edge* incumbent, const Edge& candidate, uchar to_be_pointed) {
    if(!state.is_decidable(candidate))
        return false;
    
    if(to_be_pointed != N) {
        if(candidate.x() != to_be_pointed && candidate.y() != to_be_pointed)
            return false;
        
        if(candidate.is_TH() && candidate.y() != to_be_pointed) return false;
        if(candidate.is_HT() && candidate.x() != to_be_pointed) return false;
    }

    if(incumbent == nullptr)
        return true;

    if(candidate.x() == incumbent->x() && candidate.y() == incumbent->y())
        return edge_order(candidate) < edge_order(*incumbent);

    if(instance[Heuristic] == Heuristics::Hybrid) {
        uchar inc_decided  = decided_edges(state, incumbent->x(), incumbent->y());
        uchar cand_decided = decided_edges(state, candidate.x(), candidate.y());
        
        if(inc_decided != cand_decided)
            return cand_decided < inc_decided;
    }

    uchar inc_edges  = present_edges(state, incumbent->x(), incumbent->y());
    uchar cand_edges = present_edges(state, candidate.x(), candidate.y());
    
    if(instance[Heuristic] != Heuristics::IndepsFirst && cand_edges <= inc_edges) {
        if((initial_indep[candidate.x()][candidate.y()] == 0) != (initial_indep[incumbent->x()][incumbent->y()] == 0))
            return initial_indep[candidate.x()][candidate.y()] == 0;
        
        if((initial_dep[candidate.x()][candidate.y()] > avg_dep) || (initial_dep[incumbent->x()][incumbent->y()] > avg_dep))
            return weight_ratio(candidate.x(), candidate.y()) > weight_ratio(incumbent->x(), incumbent->y());
    }

    auto& inc_weight  = bounds.info().total_weight(incumbent->x(), incumbent->y());
    auto& cand_weight = bounds.info().total_weight(candidate.x(), candidate.y());
    
    if(instance[Heuristic] != Heuristics::DependsFirst)
        if(inc_weight[1] || cand_weight[1])
            return inc_weight[1] < cand_weight[1];

    if(inc_edges != cand_edges)
        return cand_edges < inc_edges;

    return bounds.pair_score(incumbent->x(), incumbent->y()) <
        bounds.pair_score(candidate.x(), candidate.y());
}

#define consider_choosing(edgeType) \
    if(choose_edge(incumbent, Edge(x, y, edgeType), to_be_pointed)) {\
        if(incumbent != nullptr)\
            delete incumbent;\
        incumbent = new Edge(x, y, edgeType);\
    }

void BranchAndBound::solve() {
    select_next_edge(0, N);
}

#define gather_suitable_edges(edgeType) \
    if(state.is_decidable( Edge(x, y, edgeType) ))\
        edges.push_back( Edge(x, y, edgeType) );

void BranchAndBound::select_next_edge(size_t depth, uchar to_be_pointed) {
    info.visited_branches++;
    Weight lb = bounds.lowerbound(ub);
    if(lb >= ub) {
        info.good_lbs++;
        verbose("Close branch: LB % >= UB %\n", lb, ub);
        return;
    }
    
    Edge* incumbent = nullptr;
    if(instance[Heuristic] == Random) {
        std::vector<Edge> edges;
        for(uchar y = 1; y < N; y++)
            for(uchar x = 0; x < y; x++) {
                gather_suitable_edges( Edge::TH_PRESENT );
                gather_suitable_edges( Edge::HT_PRESENT );
                gather_suitable_edges( Edge::HH_PRESENT );
            }
        if(!edges.empty())
            incumbent = new Edge(edges[ rand() % edges.size() ]);
    } else
        for(uchar y = 1; y < N; y++)
            for(uchar x = 0; x < y; x++) {
                consider_choosing( Edge::TH_PRESENT );
                consider_choosing( Edge::HT_PRESENT );
                consider_choosing( Edge::HH_PRESENT );
            }
    
    if(incumbent == nullptr) {
        verbose("Close branch: No more available edge decisions.\n");
        update_ub(depth);
        if(to_be_pointed != N)
            info.symmetries1++;
        return;
    }
    
    bool edge_first = instance[Heuristic] == Random? rand() % 2:
        (instance[Heuristic] == Hybrid? add_edge_first(incumbent->x(), incumbent->y()):
         instance[Heuristic] == DependsFirst || !bounds.indep_weight(incumbent->x(), incumbent->y()));

    if(edge_first) {
        branch(*incumbent,            depth, to_be_pointed);
        branch(incumbent->negation(), depth, to_be_pointed);
    } else {
        branch(incumbent->negation(), depth, to_be_pointed);
        branch(*incumbent,            depth, to_be_pointed);
    }
    delete incumbent;
}

void BranchAndBound::branch(const Edge& decision, size_t depth, uchar to_be_pointed) {
    Bits orig_pointings = is_pointed;

    if(decision.absent()) {
        verbose("Deciding % absent.\n", decision.string());
    } else {
        verbose("Deciding % present.\n", decision.string());
        
        if(decision.is_HH() || decision.is_HT()) is_pointed.set(decision.x());
        if(decision.is_HH() || decision.is_TH()) is_pointed.set(decision.y());
    }
    
    if(retains_validity(decision, depth + 1)) {
#if DEBUG
        bounds.get_cplex().ensure_correctness([&]() {
#endif
            update(decision);
            if(decision.present() && instance[SymmetryBreaking1]) {
                to_be_pointed = N;

                if(decision.is_TH() && !is_pointed.test(decision.x())) to_be_pointed = decision.x();
                if(decision.is_HT() && !is_pointed.test(decision.y())) to_be_pointed = decision.y();

                if(to_be_pointed != N)
                    verbose("Next added edge needs to point to % (Symmetries #1).\n", (int)to_be_pointed + 1);
            }
            select_next_edge(depth + 1, to_be_pointed);
            reset(decision);
#if DEBUG
        });
#endif
    }
    
    is_pointed = orig_pointings;
}

void BranchAndBound::find_naive_UB() {

    if(instance.dependent.empty())
        update_ub();
    else for(size_t i = 0; i < instance.dependent.size(); i++) {
        const Constraint& c = instance.dependent[i];

        if(state.edge_decided(c.v1, c.v2))
            continue;
        
        if(instance[Acyclicity] && !state.retains_acyclicity(c.v1, c.v2))
            continue;
        
        if(instance[MaxDegree] != NotLimited) {
            if(state.min_degree(c.v1) >= (uchar)instance[MaxDegree]) continue;
            if(state.min_degree(c.v2) >= (uchar)instance[MaxDegree]) continue;
        }
        
        state.decide( Edge(c.v1, c.v2, Edge::TH_PRESENT) );
        update_ub();
    }

    bounds.clear_progress();
    state = Graph(instance);
    apply_edge_fixings();
}

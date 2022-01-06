#include <Graph.hpp>

Graph::Graph(node_t size):
    reach(size), edges(size, 0) {}

void Graph::add_parent(node_t y, node_t x) {
    edges[x].set(y);
    reach.set_parent(y, x);
}

void Graph::add_bidirected(node_t x, node_t y) {
    edges[x].set(y);
    edges[y].set(x);
    reach.set_bidir_edge(x, y);
}

bool Graph::has_inducing_path(node_t source, node_t target, node_t node, bits_t visited, size_t length) const {
    if(visited.test(node))
        return false;
    visited.set(node);
    
    if(node == target)
        return length >= 4;
    
    if(node != source && !reach.has(node, source) && !reach.has(node, target))
        return false;
    
    for(node_t next = 0; next < size(); next++) {
        bool to_comp =   has_edge(node, next) && node == source;
        bool from_comp = has_edge(next, node) && next == target;
        
        if(to_comp || from_comp || has_bidirected_edge(node, next))
            if(has_inducing_path(source, target, next, visited, length + 1))
                return true;
    }
    return false;
}

bool Graph::is_maximal(bits_t unassigned) const {
    for(node_t x = 0; x < size(); x++) {
        if(unassigned.test(x))
            continue;
        for(node_t y = x + 1; y < size(); y++) {
            if(unassigned.test(y))
                continue;
            if(has_edge(x, y) || has_edge(y, x) || has_bidirected_edge(x, y))
                continue;
            if(has_inducing_path(x, y, x, bits_t(0), 1))
                return false; 
        }
    }
    return true;            
}

bool Graph::is_connected(node_t node, node_t target, bits_t visited) const {
    if(node == target) return true;
    if(visited.test(node)) return false;
    visited.set(node);
    for(node_t n = 0; n < size(); n++)
        if(has_bidirected_edge(node, n) && is_connected(n, target, visited))
            return true;
    return false;
}

bool Graph::is_strongly_connected() const {
    for(node_t node = 1; node < size(); node++)
        if(!is_connected(0, node))
            return false;
    return true;
}

void Graph::output() const {
    double edge_count = 0;
    for(node_t x = 0; x < size(); x++) {
        for(node_t y = x + 1; y < size(); y++)
            if(has_bidirected_edge(x, y)) {
                print("%<>% ", x, y);
                edge_count++;
            }
        for(node_t y = x + 1; y < size(); y++) {
            if(has_edge(x, y)) { print("%->% ", x, y); edge_count++; }
            if(has_edge(y, x)) { print("%<-% ", x, y); edge_count++; }
        }
    }
    print("\n");
}

bool Graph::is_complete() const {
    for(node_t x = 0; x < size(); x++)
        for(node_t y = x + 1; y < size(); y++)
            if(!edges[x].test(y) && !edges[y].test(x))
                return false;
    return true;
}

std::vector<Scoreable> Graph::as_scoreables() const {
    std::vector<Scoreable> scoreables;
    
    bits_t items;
    for(node_t x = 0; x < size(); x++) {
        if(items.test(x))
            continue;
        bits_t comp;
        comp.set(x);
        for(node_t y = 0; y < size(); y++)
            if(is_connected(x, y))
                comp.set(y);
        items |= comp;
        
        scoreables.push_back(Scoreable(comp.count()));
        std::vector<node_t> to_index(size(), -1);
        for(node_t x = 0, index = 0; x < size(); x++)
            if(comp.test(x)) {
                scoreables.back().nodes.push_back(x);
                to_index[x] = index;
                index++;
            }

        for(node_t p = 0; p < size(); p++)
            for(node_t x = 0; x < size(); x++) {
                if(!comp.test(x))
                    continue;
                if(has_bidirected_edge(p, x) && x < p) {
                    scoreables.back().add_bidir_edge(to_index[p], to_index[x]);
                } else if(has_edge(p, x)) {
                    if(comp.test(p))
                        scoreables.back().add_inner_parent(to_index[x], to_index[p]);
                    else
                        scoreables.back().add_outer_parent(to_index[x], p);
                }
            }
    }
    
    return scoreables;
}

double Graph::score(const ICF& icf, size_t reps, size_t max_optimas) const {
    double sum = 0;
    print("\nScoring using % repetition(s) and max % local optimas per c-component:\n",
        reps, max_optimas);
    size_t most_optimas = 0;
    for(const Scoreable& item: as_scoreables()) {
        std::vector<node_t> nodes;
        for(size_t i = 0; i < item.component_size(); i++)
            nodes.push_back(item.nodes[i]);
        ICF::Stats scores = icf.score(item, reps, max_optimas);
        if(most_optimas < scores.local_optimas)
            most_optimas = scores.local_optimas;
        if(reps > 1)
            print("high % low % avg % uniq %   {%}   %\n",
                scores.highest, scores.lowest, scores.average, scores.local_optimas, to_string(nodes), item);
        else
            print("%   {%}   %\n", scores.highest, to_string(nodes), item);
        sum += scores.highest;
    }
    if(reps > 1)
        print("Most-Optimas: %\n", most_optimas);
    return sum;
}

bool Graph::has_bad_indpath(node_t source, node_t node, bits_t visited) const {
    visited.set(node);
    if(visited.count() >= 4 && !has_bidirected_edge(source, node) &&
        !has_edge(source, node) && !has_edge(node, source))
        return true;
        
    for(node_t neigh = 0; neigh < size(); neigh++) {
        if(!has_bidirected_edge(node, neigh) || visited.test(neigh))
            continue;
        if(has_bad_indpath(source, neigh, visited))
            return true;
    }
    return false;
}

bool Graph::has_bad_indpath() const {
    if(size() < 4)
        return false;
    for(node_t x = 1; x < size(); x++)
        if(has_bad_indpath(x, x, bits_t()))
            return true;
    return false;
}

Graph Graph::from_edge_matrix(std::istream& in) {
    std::vector<bits_t> edges;

    for(node_t r = 0;; r++) {
        if(in.eof() && r < edges.size())
            error("Not enough rows in the solution matrix.\n");
        if(r && r >= edges.size())
            break;

        std::string line;
        getline(in, line);
        auto parts = split(line, ',');
        if(edges.empty()) {
            if(parts.empty())
                error("Invalid solution matrix.\n");
            edges = std::vector<bits_t>(parts.size(), 0);
        }
        if(parts.size() != edges.size())
            error("The solution matrix has inconsistent dimensions.\n");

        for(node_t c = 0; c < edges.size(); c++)
            if(parts[c] == "2")
                edges[r].set(c);
            else if(parts[c] != "3" && parts[c] != "0")
                error("Invalid value '%' in the solution matrix.\n", parts[c]);
    }

    Graph graph(edges.size());
    for(node_t r = 0; r < edges.size(); r++)
        for(node_t c = 0; c < edges.size(); c++)
            if(edges[r].test(c) && edges[c].test(r))
                graph.add_bidirected(r, c);
            else if(edges[r].test(c))
                graph.add_parent(c, r);
    return graph;
}

void add_edge(Graph& graph, const std::string& line, size_t node_count) {
    std::vector<size_t> vars;
    for(const std::string& item: split(line, ' ')) {
        if(item.length() < 2)
            continue;
        if(item[0] == 'C') {
            std::string number = item.substr(1, item.length());
            std::stringstream stream(number);
            size_t index;
            stream >> index;
            if(index == 0 || index > node_count)
                error("Invalid variable '%'?\n", item);
            vars.push_back(index - 1);
        }
    }
    if(vars.size() < 2)
        error("One of the edge definitions has less than 2 variables.\n")
    if(vars.size() > 2)
        print("\n** Warning: One of the edge definitions has more than 2 variables! **\n");

    if(line.find("-->") != std::string::npos)
        graph.add_parent(vars[1], vars[0]);
    else if(line.find("<--") != std::string::npos)
        graph.add_parent(vars[0], vars[1]);
    else if(line.find("<->") != std::string::npos)
        graph.add_bidirected(vars[0], vars[1]);
    else
        error("Invalid edge in the input file.\n");
}

Graph Graph::from_ccd_output(std::istream& in) {
    size_t node_count = 0;
    do {
        if(in.eof())
            error("The graph file does not express variables.\n");
        std::string line;
        getline(in, line);
        
        auto pieces = split(line, ';');
        node_count = pieces.size();
        for(size_t i = 0; i < pieces.size(); i++)
            if(pieces[i] != 'C' + to_string(i + 1)) {
                node_count = 0;
                break;
            }
    } while(!node_count);
    
    Graph graph(node_count);
    size_t numbering = 1;
    while(!in.eof()) {
        std::string line;
        getline(in, line);
        if(line.find(to_string(numbering) + ". ") == 0) {
            add_edge(graph, line, node_count);
            numbering++;
        }
    }
    return graph;
}

size_t deduce_node_count(const std::string& path) {
    std::string fname = split(path, '/').back();
    
    size_t count = 0;
    if(fname.find("-n") != std::string::npos) {
        auto parts = split(fname, '-');
        if(parts.size() < 4 || parts[1].length() < 2 || parts[1][0] != 'n')
            error("Something wrong with the file path. (1)\n");
        std::stringstream stream( parts[1].substr(1, parts[1].length()) );
        stream >> count;
    } else {
        auto parts = split(fname, '_');
        if(parts.size() < 4 || parts[1] == "")
            error("Something wrong with the file path. (2)\n");
        std::stringstream stream( parts[1] );
        stream >> count;
    }
    return count;
}

void read_edge(Graph& graph, const std::string& edge, size_t node_count) {
    std::string s1, s2;
    for(size_t i = 0; i < edge.length() && edge[i] >= '0' && edge[i] <= '9'; i++)
        s1 = s1 + edge[i];
    for(size_t i = edge.length() - 1; i > 0 && edge[i] >= '0' && edge[i] <= '9'; i--)
        s2 = edge[i] + s2;
    node_t v1, v2;
    std::stringstream st1(s1); st1 >> v1;
    std::stringstream st2(s2); st2 >> v2;
    
    if(v1 >= node_count || v2 >= node_count)
        error("Invalid variable in an edge definition.\n");
    
    if(edge.find("<-") != std::string::npos)
        graph.add_parent(v1, v2);
    else if(edge.find("->") != std::string::npos)
        graph.add_parent(v2, v1);
    else if(edge.find("<>") != std::string::npos)
        graph.add_bidirected(v1, v2);
    else
        error("Unrecognized edge type in the input (%).\n", edge);
}

Graph Graph::from_own_output(std::istream& in) {
    std::vector<std::string> lines;
    while(!in.eof()) {
        lines.push_back("");
        getline(in, lines.back());
    }
    if(lines.size() < 3)
        error("Not enough lines in the input graph.\n");
    
    if(lines[0].find("Arguments given:") == std::string::npos)
        error("The input is not in correct format.\n");
    
    size_t node_count = 0;
    for(const std::string& item: split(lines[0], ' '))
        if(item.find(".bic") != std::string::npos)
            node_count = deduce_node_count(item);
    if(node_count < 1 || node_count > 32)
        error("Could not determine the node count.\n");

    Graph graph(node_count);
    for(size_t i = lines.size() - 1; i > 0; i--)
        if(lines[i - 1].find("= Optimal Solution Found") != std::string::npos) {
            for(const std::string& edge: split(lines[i], ' '))
	        if(edge.length() > 1)
                    read_edge(graph, edge, node_count);
            break;
        }
    return graph;
}

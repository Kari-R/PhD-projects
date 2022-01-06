#include <Solution.hpp>

Solution::Solution(node_t size, score_t starting_score): reach(size),
    edges(size, 0), score_(starting_score) {}

Solution::Solution(node_t size): Solution(size, 0) {}

void Solution::add_parent(node_t y, node_t x) {
    edges[x].set(y);
    reach.set_parent(y, x);
}

void Solution::add_bidirected(node_t x, node_t y) {
    edges[x].set(y);
    edges[y].set(x);
    reach.set_bidir_edge(x, y);
}

void Solution::add(const Comparent& target) {
    const Set& comp = target.component();
    
    for(node_t x = 0; x < comp.size(); x++) {
        for(node_t parent: target.parents(comp[x]).contents())
            add_parent(comp[x], parent);

        for(node_t y = x + 1; y < comp.size(); y++)
            if(target.has_edge(comp[x], comp[y]))
                add_bidirected(comp[x], comp[y]);
    }
    
    target.update_reach(reach);
    score_ += target.score();
}

bool Solution::has_inducing_path(node_t source, node_t target, node_t node, bits_t visited, size_t length) const {
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

bool Solution::is_maximal(bits_t unassigned) const {
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

void Solution::build_c_component(node_t x, bits_t& comp) const {
    if(comp.test(x)) return;
    comp.set(x);
    for(node_t y = 0; y < size(); y++)
        if(has_bidirected_edge(x, y))
            build_c_component(y, comp);
}

void Solution::output(const std::string& name) const {
    double edge_count = 0;
    size_t bidir_count = 0;
    for(node_t x = 0; x < size(); x++) {
        for(node_t y = x + 1; y < size(); y++)
            if(has_bidirected_edge(x, y)) {
                print("%<>% ", x, y);
                edge_count += 2;
		bidir_count++;
            }
        for(node_t y = x + 1; y < size(); y++) {
            if(has_edge(x, y)) { print("%->% ", x, y); edge_count += 2; }
            if(has_edge(y, x)) { print("%<-% ", x, y); edge_count += 2; }
        }
    }
    print("\n%-Score: %\n%-Degree: %\n",
        name, -score(), name, double(int(edge_count / size() * 100 + 0.5)) / 100);
    size_t max_comp = 0;
    for(node_t x = 0; x < size(); x++) {
        bits_t comp;
        build_c_component(x, comp);
        if(max_comp < comp.count())
            max_comp = comp.count();
    }
    print("%-Max-Comp: %\n%-Bidir-Edges: %\n",
	name, max_comp, name, bidir_count);
}

bool Solution::is_connected(node_t node, node_t target, bits_t visited) const {
    if(node == target)
        return true;
    if(visited.test(node))
        return false;
    visited.set(node);
    for(node_t n = 0; n < size(); n++)
        if(has_bidirected_edge(node, n) && is_connected(n, target, visited))
            return true;
    return false;
}

#include <Solution.hpp>
#include <debug.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <map>

#define N vertexes.size()

template<size_t Columns>
struct Table {
    std::vector<std::string> columns[Columns];

    template<size_t Column, typename... Stuff>
    Table& column(const char * f, Stuff... stuff) {
        columns[Column].push_back(format(f, stuff...));
        return * this;
    }
    
    Table& line() { return * this; }
    
    size_t column_width(size_t column) const {
        size_t width = 0;
        for(size_t i = 0; i < columns[column].size(); i++)
            if(width < columns[column][i].length())
                width = columns[column][i].length();
        return width;
    }
    
    std::string justify(std::string string, size_t width) {
        string.reserve(width);
        while(string.length() < width)
            string += " ";
        return string;
    }
    
    std::vector<std::string> get_lines() {
        size_t widths[Columns];
        for(size_t i = 0; i < Columns; i++)
            widths[i] = column_width(i);

        std::vector<std::string> lines;
        for(size_t i = 0; i < columns[0].size(); i++) {
            std::string line;
            for(size_t j = 0; j < Columns; j++) {
                if(j > 0)
                    line += " | ";
                line += justify(columns[j][i], widths[j]);
            }
            lines.push_back(line);
        }
        return lines;
    }
    
    void output() {
        for(const std::string& line: get_lines())
            std::cout << line << std::endl;
    }
};

uint find_root(const Solution& state, const Domains& domains, uint var, std::vector<uint>& roots) {
    if(roots[var] != domains.size())
        return roots[var];
    
    const ParentSet& parents = domains[var][state[var].parentset];
    if(parents.content.none())
        return roots[var] = var;
    
    return roots[var] = find_root(state, domains, parents.list[0], roots);
}

void find_order(const Solution& state, const Domains& domains, std::vector<uint>& order) {
    std::vector<uint> roots(state.vertexes.size(), domains.size());
    for(uint var = 0; var < state.vertexes.size(); var++) {
        if(state.contains(var))
            find_root(state, domains, var, roots);
        order.push_back(var);
    }
    
    std::sort(order.begin(), order.end(),
    [&](uint a, uint b) {
        if(!state.contains(a)) return false;
        if(!state.contains(b)) return true;
        if(roots[a] != roots[b])
            return roots[a] < roots[b];
        if(state[a].depth != state[b].depth)
            return state[a].depth < state[b].depth;
        return a < b;
    });
}

void Solution::print(const Domains& domains) const {    
    std::vector<uint> order;
    find_order(* this, domains, order);
    
    Table<3> table;
    for(uint var : order) {
        if(!assign_mask.test(var))
            continue;
        
        std::string tree;
        for(uint i = 1; i < vertexes[var].depth; i++)
            tree += "  ";
        if(vertexes[var].depth > 0)
            tree += "+-- ";
        
        table.line()
            .column<0>(" %", vertexes[var].depth)
            .column<1>("%p_% = [%]", tree, var, domains[var][vertexes[var].parentset])
            .column<2>("%", to_string(domains[var][vertexes[var].parentset].score));
    }
    table.output();
    std::cout << "Score: " << to_string(score) << std::endl;
}

void Solution::set_parents(uint var, const ParentSet& parents) {
    assign_mask.set(var, true);
    skeleton[var][var] = true;
    for(uint i = 0; i < parents.list.size(); i++) {
        skeleton[var][parents[i]] = true;
        skeleton[parents[i]][var] = true;
    }
}

void Solution::reset_parents(uint var) {
    assign_mask.set(var, false);
    skeleton[var].reset();
    for(uint v = 0; v < N; v++)
        skeleton[v][var] = false;
}

size_t calculate_depth(const ArcMatrix& matrix, Solution& solution, uint var) {
    if(solution[var].depth < matrix.size())
        return solution[var].depth;
    
    solution[var].depth = 0;
    for(uint parent = 0; parent < matrix.size(); parent++) {
        if(!matrix[var][parent] || var == parent)
            continue;
        size_t depth = calculate_depth(matrix, solution, parent) + 1;
        if(solution[var].depth < depth)
            solution[var].depth = depth;
    }
    return solution[var].depth;
}

Solution Solution::construct(
const Domains& domains,
const std::vector<DomainLookup>& domain_lookup,
const Network& network) {

    Solution solution(network.score, domains.size(), domains.size());
    
    std::vector<uint> layers[domains.size() + 1];
    for(uint var = 0; var < network.matrix.size(); var++) {
        size_t depth = calculate_depth(network.matrix, solution, var);
        layers[depth].push_back(var);
    }
    
    for(uint var = 0; var < network.matrix.size(); var++) {
        std::bitset<32> bitset = network.matrix[var];
        bitset.reset(var);

        solution[var].parentset = domain_lookup[var].at(bitset);
        solution.set_parents(var, domains[var][solution[var].parentset]);
    }
    return solution;
}

const size_t letters = ('z' - 'a');
const size_t symbols = letters * 2 + 10;

void add_symbol(std::string& string, size_t i) {
    if(i < letters)
        string += char('a' + i);
    else if(i < letters * 2)
        string += char('A' + (i - letters));
    else
        string += char('0' + (i - letters * 2));
}

std::string Solution::fingerprint() const {
    size_t hash = NetworkHash()(skeleton);

    size_t multiplier = 1;
    while(multiplier * symbols <= hash && multiplier < multiplier * symbols)
        multiplier *= symbols;
    
    std::string string;
    while(hash >= symbols) {
        size_t digit = 0;
        while(multiplier * (digit + 1) <= hash)
            digit++;
        add_symbol(string, digit);
        hash -= multiplier * digit;
        multiplier /= symbols;
    }
    if(hash > 0)
        add_symbol(string, hash);
    return string;
}
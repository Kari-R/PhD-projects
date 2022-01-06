#include <Instance.hpp>
#include <Settings.hpp>
#include <algorithm>

template<typename T>
void sort(T& v) {
    std::sort(v.begin(), v.end(),
    [](const Constraint& a, const Constraint& b) {
        return a.weight > b.weight;
    });
}

Instance::Instance(const Settings& settings, std::istream& in):
settings(settings), largest_var(0)  {

    std::vector<std::string> lines;
    smallest_var = 255;
    while(!in.eof()) {
        std::string line;
        std::getline(in, line);
        if(line.length() >= 5) {
            lines.push_back(line);
            std::stringstream stream(line);
            size_t _, v1, v2;
            stream >> _ >> _ >> v1 >> v2;
            smallest_var = std::min(smallest_var, (uchar)std::min(v1, v2));
        }
    }
    
    print("Indexing variables from %.\n", (int)smallest_var);
    for(const std::string& line: lines)
        parse_line(line);
    finalize();
}

void Instance::finalize() {
    for(auto it: lookup) {
        if(it.second.independence)
            independent.push_back(it.second);
        else
            dependent.push_back(it.second);
        
        Bits intervent = it.second.intervent;
        auto it2 = by_intervent.find(intervent.to_ulong());
        if(it2 == by_intervent.end())
            it2 = by_intervent.insert(by_intervent.begin(),
                std::make_pair(intervent.to_ulong(), std::vector<Constraint::Id>()));
        it2->second.push_back(it.second.ident());
    }
    
    sort(independent);
    sort(dependent);
    
    verify_settings();
}

void Instance::verify_settings() {
    if(size() < settings[MaxVars])
        print_error("Variable limit is unnecessarily high.\n");
    
    if(/*settings[MaxCond] != NotLimited &&*/ settings[MaxCond] + 2 > size())
        print_error("Condition limit is unnecessarily high.\n");

    if(settings[MaxDegree] != NotLimited && settings[MaxDegree] > 3 * (size() - 1))
        print_error("Degree limit is unnecessarily high.\n");
}

void Instance::parse_line(const std::string& line) {
    std::stringstream stream(line);
    size_t separated, weight, v1, v2, cond, intervention;
    stream >> separated >> weight >> v1 >> v2 >> cond >> intervention;
    v1 -= smallest_var;
    v2 -= smallest_var;

    if(settings[MaxVars] != NotLimited) {
        if(v1 >= (size_t)settings[MaxVars]) return;
        if(v2 >= (size_t)settings[MaxVars]) return;
        if(cond >= ((size_t(1)) << settings[MaxVars])) return;
    }

    if(settings[MaxCond] != NotLimited && Bits(cond).count() > (size_t)settings[MaxCond])
        return;

    add(separated, v1, v2, weight, cond, intervention);
}

void Instance::add(bool separated, uchar v1, uchar v2, Weight weight, Bits condition, Bits intervent) {    
    Constraint c(v1, v2, condition, separated, weight, intervent);
    
    auto it = lookup.find(c.ident());
    if(it == lookup.end()) {
        lookup.insert(std::make_pair(c.ident(), c));
        largest_var = std::max(largest_var, std::max(v1, v2));
        return;
    }
    
    Constraint c2 = it->second;
    if(c.independence == c2.independence)
        it->second.weight = c.weight + c2.weight;
    else if(c.weight <= c2.weight) {
        it->second.weight = c2.weight - c.weight;
        it->second.independence = c2.independence;
    } else if(c2.weight < c.weight) {
        it->second.weight = c.weight - c2.weight;
        it->second.independence = c.independence;
    }
    print("** Warning: Merging constraints: **\n"
            "   (1) % and\n"
            "   (2) %\n"
            "  into %\n", c, c2, it->second);
}

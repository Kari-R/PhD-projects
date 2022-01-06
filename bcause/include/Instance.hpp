#pragma once

#include <useful.hpp>
#include <Settings.hpp>
#include <unordered_map>
#include <iostream>
#include <list>
#include <vector>
#include <map>

struct Constraint {
    typedef unsigned long long Id;
    
    uchar v1, v2;
    Bits condition;
    bool independence;
    Weight weight;
    Bits intervent;

    Constraint(uchar v1, uchar v2, Bits condition, bool independency = true,
        Weight weight = INF, Bits intervent = 0):
    v1(v1), v2(v2), condition(condition), independence(independency),
    weight(weight), intervent(intervent) {
        if(v1 > v2)
            std::swap(v1, v2);
    }
    
    inline Constraint with_intervention(Bits intervent) {
        Constraint copy(*this);
        copy.intervent = intervent;
        return copy;
    }
    
    inline bool operator==(const Constraint& c) const {
        return v1 == c.v1 && v2 == c.v2 && condition == c.condition && intervent == c.intervent;
    }
    
    inline bool operator<(const Constraint& c) const {
        if(v1 != c.v1) return v1 < c.v1;
        if(v2 != c.v2) return v2 < c.v2;
        if(intervent != c.intervent)
            return intervent.to_ulong() < c.intervent.to_ulong();
        return condition.to_ulong() < c.condition.to_ulong();
    }
    
    friend std::ostream& operator<<(std::ostream&, const Constraint&);
    
    inline static Constraint::Id ident(uchar v1, uchar v2, Bits condition, Bits intervent = 0) {
        if(v1 > v2)
            return ident(v2, v1, condition, intervent);
        
        Id id = 0;
        id |= v1; id = id << 5;
        id |= v2; id = id << 27;
        id |= condition.to_ulong(); id = id << 27;
        id |= intervent.to_ulong();
        return id;
    }
    
    inline Constraint::Id ident() const {
        return Constraint::ident(v1, v2, condition, intervent);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Constraint& c) {
    os << format("% % % | %", (int)c.v1,
        c.independence? "_|_": "_N_", (int)c.v2,
        to_string(c.condition));
    if(c.intervent.any())
        os << " || " << to_string(c.intervent);
    if(c.weight != INF)
        os << " (weight: " << c.weight << ")";
    return os;
}

class Instance {
public:    
    std::vector<Constraint> independent, dependent;
    std::unordered_map<size_t, std::vector<Constraint::Id> > by_intervent;
    
    inline char size() const { return largest_var + 1; }
    
    Instance(const Settings&, std::istream&);
    
    void add(bool, uchar, uchar, Weight, Bits, Bits);
    
    inline bool contains(uchar v1, uchar v2, Bits condition, Bits intervent) const {
        return lookup.find(Constraint::ident(v1, v2, condition, intervent)) != lookup.end();
    }
    
    inline Settings::ArgT operator[](Argument arg) const {
        return settings[arg];
    }
    
    inline uchar variable_indexing() const { return smallest_var; }
    
    void parse_line(const std::string&);
    void finalize();
    void verify_settings();
    
    inline const Constraint& get(Constraint::Id id) const {
        auto it = lookup.find(id);
        ensure(it != lookup.end());
        return it->second;
    }
    
    inline const Constraint& get(uchar v1, uchar v2, Bits condition, Bits intervent) const {
        return get(Constraint::ident(v1, v2, condition, intervent));
    }
    
    inline const std::string& fixed_edges() const { return settings.fixed_edges(); }
    inline Weight starting_ub() const { return settings.starting_ub(); }
    
private:
    const Settings& settings;
    uchar largest_var, smallest_var;
    std::unordered_map<Constraint::Id, Constraint> lookup;
};

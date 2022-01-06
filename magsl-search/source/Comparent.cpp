#include <Comparent.hpp>
#include <list>

Comparent::Comparent(bits_t bits, score_t score_):
        score_(score_), comp(bits), absent_edges(comp.size(), 0) {
    size_t largest = 0;
    for(node_t node: comp.contents())
        if(largest < node) largest = node;
    node_to_index = std::vector<node_t>(largest + 1, 0);
    for(size_t i = 0; i < comp.size(); i++)
        node_to_index[comp[i]] = i;
}

void Comparent::add_parents(bits_t set) {
    ensure(psets.size() < comp.size());
    psets.push_back(set);
}

void Comparent::add_missing_edge(bits_t edge) {
    ensure(edge.count() == 2 || edge.count() == 0);
    missing_edge = edge;
    
    for(node_t node: comp.contents())
        if(edge.test(node))
            absent_edges[ node_to_index[node] ] |= edge;
}

bool Comparent::has_edge(node_t x, node_t y) const {
    return x != y && !absent_edges[ node_to_index[x] ].test(y);
}

void Comparent::update_reach(Reach& reach) const {
    for(node_t x = 0; x < component().size(); x++) {
        for(node_t parent: parents(comp[x]).contents())
            reach.set_parent(comp[x], parent);

        for(node_t y = x + 1; y < component().size(); y++)
            if(has_edge(comp[x], comp[y]))
                reach.set_bidir_edge(comp[x], comp[y]);
    }            
}

std::ostream& operator<<(std::ostream& os, const Comparent& c) {
    
    if(c.component().size() <= 3) {
        std::list<node_t> nodes, missing;
    
        for(node_t node: c.component().contents()) {
            if(c.get_absent_edges(node).any())
                missing.push_back(node);
            else
                nodes.push_back(node);
        }
        if(!missing.empty()) {
            nodes.push_back(missing.front());
            nodes.push_front(missing.back());
        }
        auto it = nodes.begin();
        for(node_t i = 0; i < nodes.size(); i++, it++) {
            if(i > 0) os << "<>";
            os << *it;
        }
        if(missing.empty() && c.component().size() == 3)
            os << "<>" << nodes.front();
    }
    else {
        bool printed = false;
        for(node_t x: c.component().contents())
            for(node_t y: c.component().contents())
                if(c.has_edge(x, y)) {
                    if(printed) os << ", ";
                    os << x << "<>" << y;
                    printed = true;
                }
    }

    for(node_t node: c.component().contents())
        os << ";  " << node << "<-" << c.parents(node);
    return os << ";  " << c.score_;
}

void Comparent::output_to(std::ostream& os) const {
    os << std::setprecision(FP_PRECISION) <<
        score_ << " " << component().bitset().to_ulong();
    for(const Set& pset: psets)
        os << " " << pset.bitset().to_ulong();
    os << " " << missing_edge.to_ulong() << std::endl;
}

#define PARENTS_START ( 2 )
#define ABSENTS_START ( PARENTS_START + bits_t(sets[1]).count() )

Comparent::Key::Key(const Comparent& c) {
     sets.push_back(0);
     sets.push_back(c.comp.bitset().to_ulong());
     for(const Set& set: c.psets)
         sets.push_back(set.bitset().to_ulong());
     
     for(node_t i = 0; i < c.absent_edges.size(); i++)
         sets.push_back( c.absent_edges[i].to_ulong() );
 }

Comparent::Key::Key(bits_t comp, const Comparent& refr) {
    sets.push_back(0);
    sets.push_back(comp.to_ulong());

    for(size_t node: refr.component().contents())
        if(comp.test(node))
            sets.push_back( refr.parents(node).bitset().to_ulong() );
    
    for(size_t i = 0; i < refr.component().size(); i++)
        if(comp.test( refr.component()[i] ))
            sets.push_back( refr.absent_edges[i].to_ulong() );
}

Comparent::Key& Comparent::Key::add_missing_edge(const Comparent& c, node_t x, node_t y) {
    ensure(x != y);    
    ensure( (sets[ABSENTS_START + c.order(x)] & (size_t(1) << y)) == 0 );
    ensure( (sets[ABSENTS_START + c.order(y)] & (size_t(1) << x)) == 0 );
    
    sets[ABSENTS_START + c.order(x)] |= (size_t(1) << y);
    sets[ABSENTS_START + c.order(y)] |= (size_t(1) << x);
    return *this;
}

Comparent::Key& Comparent::Key::remove_missing_edge(const Comparent& c, node_t x, node_t y) {
    ensure(x != y);
    ensure( (sets[ABSENTS_START + c.order(x)] & (size_t(1) << y)) != 0 );
    ensure( (sets[ABSENTS_START + c.order(y)] & (size_t(1) << x)) != 0 );

    sets[ABSENTS_START + c.order(x)] &= ~(size_t(1) << y);
    sets[ABSENTS_START + c.order(y)] &= ~(size_t(1) << x);
    return *this;
}

Comparent::Key& Comparent::Key::set_parents(node_t i, bits_t parents) {
    sets[PARENTS_START + i] = parents.to_ulong();
    return *this;
}

bool Comparent::Key::operator==(const Key& k) const {
    if(sets.size() != k.sets.size())
        return false;
    for(size_t i = 0; i < sets.size(); i++)
        if(sets[i] != k.sets[i])
            return false;
    return true;
}

void Comparent::Key::debug() {
    for(size_t e: sets)
        print("      [%]\n", bits_t(e));
}

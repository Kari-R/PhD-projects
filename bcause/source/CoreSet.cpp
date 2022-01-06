#include <CoreSet.hpp>
#include <CPLEX.hpp>

CoreSet::CoreSet(const Instance& instance):
    instance(instance),
    unsat_deps(instance.size(),
        std::vector<size_t>(instance.size(), 0) )
{ open_scope(); }

void CoreSet::add(const std::list<Constraint>& core) {
    cores.push_back(core);
    
    for(const Constraint& item: core) {
        auto it = lookup.find(item.ident());
        if(it == lookup.end())
            it = lookup.insert(lookup.begin(),
                std::make_pair(item.ident(), std::vector<Core*>() ));
        it->second.push_back(&cores.back());
    }
}

const Constraint& CoreSet::identify_the_remaining(const Core& core) const {
    for(const Constraint& item: core.items)
        if(status.find(item.ident()) == status.end())
            return item;
    
    print("Error: Everything identified in an UNSAT core?\n");
    std::exit(-1);
}

void CoreSet::mark_satisfied(const Constraint& c) {
    auto it = lookup.find( c.ident() );
    if(it == lookup.end())
        return;
    
    status.insert(std::make_pair(c.ident(), CPLEX::Sat));
    
    for(Core* core: it->second) {
        core->undefs_left--;
        if(core->undefs_left == 1 && core->unsats == 0) {

            const Constraint& remain = identify_the_remaining(*core);
            propagation_levels.back().push_back(&remain);
            propagated_unsats.insert(remain.ident());
            
            if( !instance.get(remain.v1, remain.v2, remain.condition, remain.intervent).independence )
                unsat_deps[remain.v1][remain.v2]++;
        }
    }
}

void CoreSet::mark_unsatisfied(const Constraint& c) {
    auto it = lookup.find( c.ident() );
    if(it == lookup.end())
        return;
    
    status.insert(std::make_pair(c.ident(), CPLEX::Unsat));
    
    for(Core* core: it->second) {
        core->unsats++;
        core->undefs_left--;
    }
}

void CoreSet::reset(const Constraint& c) {
    auto it = lookup.find( c.ident() );
    if(it == lookup.end())
        return;
    
    auto it2 = status.find( c.ident() );
    bool was_satisfied = it2->second == CPLEX::Sat;
    status.erase(it2);
    
    for(Core* core: it->second) {
        if(!was_satisfied)
            core->unsats--;
        core->undefs_left++;
    }
}

void CoreSet::close_scope() {

    while(!propagation_levels.back().empty()) {
        const Constraint* c = propagation_levels.back().back();

        propagated_unsats.erase(c->ident());
        propagation_levels.back().pop_back();
        
        if( !instance.get(c->v1, c->v2, c->condition, 0).independence ) {
            ensure( unsat_deps[c->v1][c->v2] > 0 );
            unsat_deps[c->v1][c->v2]--;
        }
    }
    propagation_levels.pop_back();
}

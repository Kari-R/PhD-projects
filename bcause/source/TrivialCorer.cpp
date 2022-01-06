#include <TrivialCorer.hpp>

#define is_independent(v1, v2, cond, interv) \
    instance.contains(v1, v2, cond, interv) &&\
    instance.get(v1, v2, cond, interv).independence

#define is_dependent(v1, v2, cond, interv) \
    instance.contains(v1, v2, cond, interv) &&\
    !instance.get(v1, v2, cond, interv).independence

#define CONSTRAINT(x, y, S, I) \
    Constraint(x, y, S).with_intervention(I)

#define N instance.size()

TrivialCorer::TrivialCorer(const Instance& instance):
instance(instance), found(0), fed_to_cplex(0), noncores_fed(0) {}

inline Bits operator+(Bits bits, uchar value) {
    return bits.set(value);
}

void TrivialCorer::consider1(Bits S, Bits I, uchar x, uchar y, uchar z) {
    if( is_independent(x, z, S,     I) &&
        is_independent(x, y, S,     I) &&
        is_dependent(x, z,   S + y, I)) {

        add_core({
            CONSTRAINT(x, z, S,     I),
            CONSTRAINT(x, y, S,     I),
            CONSTRAINT(x, z, S + y, I) });
    }
}

void TrivialCorer::consider2(Bits S, Bits I, uchar x, uchar y, uchar z) {
    if(x >= y) return;
    if( is_dependent(x, z,   S,     I) &&
        is_dependent(y, z,   S,     I) &&
        is_independent(x, y, S,     I) &&
        is_independent(x, y, S + z, I)) {

        add_core({
            CONSTRAINT(x, z, S,     I),
            CONSTRAINT(y, z, S,     I),
            CONSTRAINT(x, y, S,     I),
            CONSTRAINT(x, y, S + z, I) });
    }
}

void TrivialCorer::consider3(Bits S, Bits I, uchar x, uchar y, uchar z) {
    if( is_dependent(x, z,   S + y,  I) &&
        is_dependent(y, z,   S + x,  I) &&
        is_independent(x, y, S,      I) &&
        is_independent(x, y, S + z,  I)) {

        add_core({
            CONSTRAINT(x, z, S + y, I),
            CONSTRAINT(y, z, S + x, I),
            CONSTRAINT(x, y, S,     I),
            CONSTRAINT(x, y, S + z, I) });
    }
}

void TrivialCorer::consider4(Bits S, Bits I, uchar x, uchar y, uchar z, uchar w) {
    if( is_dependent(y, z,   S,         I) &&
        is_dependent(x, z,   S,         I) &&
        is_independent(z, w, S + x + y, I) &&
        is_independent(x, y, S + z,     I) &&
        is_independent(x, y, S + w,     I)) {

        add_core({
            CONSTRAINT(y, z, S,         I),
            CONSTRAINT(x, z, S,         I),
            CONSTRAINT(z, w, S + x + y, I),
            CONSTRAINT(x, y, S + z,     I),
            CONSTRAINT(x, y, S + w,     I) });
    }
}

void TrivialCorer::consider5(Bits S, Bits I, uchar x, uchar y, uchar z, uchar w) {
    if( is_dependent(y, z,   S,     I) &&
        is_dependent(x, z,   S,     I) &&
        is_independent(x, y, S + z, I) &&
        is_independent(z, w, S + y, I) &&
        is_independent(x, y, S + w, I)) {

        add_core({
            CONSTRAINT(y, z, S,     I),
            CONSTRAINT(x, z, S,     I),
            CONSTRAINT(x, y, S + z, I),
            CONSTRAINT(z, w, S + y, I),
            CONSTRAINT(x, y, S + w, I) });
    }
}

void TrivialCorer::consider6(Bits S, Bits I, uchar x, uchar y, uchar z, uchar w) {
    if( is_dependent(x, y,   S + z,     I) &&
        is_dependent(y, z,   S + x + w, I) &&
        is_dependent(w, y,   S + z,     I) &&
        is_independent(w, x, S + z + y, I) &&
        is_independent(x, z, S + w,     I)) {

        add_core({
            CONSTRAINT(x, y, S + z,     I),
            CONSTRAINT(y, z, S + x + w, I),
            CONSTRAINT(w, y, S + z,     I),
            CONSTRAINT(w, x, S + z + y, I),
            CONSTRAINT(x, z, S + w,     I) });
    }
}

void TrivialCorer::consider7(Bits S, Bits I, uchar x, uchar y, uchar z, uchar w) {
    if( is_dependent(x, y,   S + z,     I) &&
        is_dependent(y, z,   S + x + w, I) &&
        is_dependent(w, y,   S,         I) &&
        is_independent(w, x, S + y,     I) &&
        is_independent(x, z, S + w,     I)) {

        add_core({
            CONSTRAINT(x, y, S + z,     I),
            CONSTRAINT(y, z, S + x + w, I),
            CONSTRAINT(w, y, S,         I),
            CONSTRAINT(w, x, S + y,     I),
            CONSTRAINT(x, z, S + w,     I) });
    }
}

void TrivialCorer::gather(Bits intervent) {
    for(uchar x = 0; x < N; x++) {
        for(uchar y = 0; y < N; y++) {
            if(x == y) continue;
            for(uchar z = 0; z < N; z++) {
                if(z == x || z == y) continue;
                
                for(size_t S = 0; S < (size_t(1) << N); S++) {
                    if(Bits(S).test(x) || Bits(S).test(y) || Bits(S).test(z))
                        continue;
                    
                    consider1(S, intervent, x, y, z);
                    consider2(S, intervent, x, y, z);
                    consider3(S, intervent, x, y, z);
                    
                    for(uchar w = 0; w < N; w++) {
                        if(w == x || w == y || w == z || Bits(S).test(w)) continue;

                        consider4(S, intervent, x, y, z, w);
                        consider5(S, intervent, x, y, z, w);
                        consider6(S, intervent, x, y, z, w);
                        consider7(S, intervent, x, y, z, w);
                    }
                }
            }
        }
    }
}

void TrivialCorer::gather_special(CPLEX& cplex, const std::vector<Constraint::Id>& constraints, Bits intervent) {
    for(const Constraint::Id& id: constraints) {
        Constraint c1 = instance.get(id);

        for(auto jt = instance.by_intervent.begin(); jt != instance.by_intervent.end(); jt++) {
            if(Bits(jt->first) == intervent || (intervent & Bits(jt->first)) != intervent)
                continue;
            
            if(!instance.contains(c1.v1, c1.v2, c1.condition, jt->first))
                continue;
            
            Constraint c2 = instance.get(c1.v1, c1.v2, c1.condition, jt->first);
            cplex.add_intervention_constraint(c1, c2);
            noncores_fed++;
        }
    }
}

void TrivialCorer::gather_special(CPLEX& cplex) {
    if(instance[NoncoreConstraints])
        for(auto it = instance.by_intervent.begin(); it != instance.by_intervent.end(); it++)
            gather_special(cplex, it->second, it->first);
}

void TrivialCorer::gather() {
    for(auto it = instance.by_intervent.begin(); it != instance.by_intervent.end(); it++)
        gather(it->first);
}

void TrivialCorer::feed_to(CPLEX& cplex, CoreSet& cores, size_t max_cores) {
    for(size_t size = 3; size <= 5; size++) {
        for(const auto& core: cores_by_size[size]) {
            if(fed_to_cplex >= max_cores)
                break;
            
            cplex.add_core(core);
            cores.add(core);
            fed_to_cplex++;
        }
    }
}

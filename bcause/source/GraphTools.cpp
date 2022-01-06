#include <GraphTools.hpp>

#define N completion.size()

template<bool SeprType = GraphTools::D_SEPARATION>
class HelperClass {
public:
    inline bool edge(uchar v1, uchar v2) const {
        return completion.edges(v1, v2);
    }

    inline bool biedge(uchar v1, uchar v2) const {
        return completion.biedges(v1, v2);
    }
    
    #define EDGE(x, y)   (edge(x, y) && !c.intervent.test(y))
    #define BIEDGE(x, y) (biedge(x, y) &&\
        !c.intervent.test(x) && !c.intervent.test(y))
    
    HelperClass(const Completion& completion):
        completion(completion) {}
    
    inline bool sigma_rule(uchar center, uchar entry, const Constraint& c) {
        return SeprType == GraphTools::SIGMA_SEPARATION &&
            c.condition.test(center) && EDGE(center, entry) &&
            has_directed_path(entry, center, c.intervent);
    }

    inline bool enter_by_outward_edge(uchar neighbour, uchar node, const Constraint& c) {
        if(!c.condition.test(neighbour) && handle(false, neighbour, c))
            return true;
        else if(sigma_rule(neighbour, node, c) && handle(true, neighbour, c))
            return true;
        return false;
    }

    bool pass_collider(uchar node, const Constraint& c) {
        for(uchar neighbour = 0; neighbour < N; neighbour++) {
            if(EDGE(neighbour, node) && enter_by_outward_edge(neighbour, node, c))
                return true;
            
            if(EDGE(node, neighbour) && sigma_rule(node, neighbour, c) && handle(true, neighbour, c))
                return true;
            
            if(BIEDGE(neighbour, node) && handle(true, neighbour, c))
                return true;
        }
        return false;
    }

    bool pass_noncollider(bool pointed, uchar node, const Constraint& c) {
        for(uchar neighbour = 0; neighbour < N; neighbour++)
            if(EDGE(node, neighbour) && handle(true, neighbour, c))
                return true;
        
        if(pointed) return false;

        for(uchar neighbour = 0; neighbour < N; neighbour++) {
            if(EDGE(neighbour, node) && enter_by_outward_edge(neighbour, node, c))
	        return true;
            
            if(BIEDGE(neighbour, node) && handle(true, neighbour, c))
                return true;
        }
        return false;
    }

    bool handle(bool pointed, uchar node, const Constraint& c) {
        if(node == c.v1 || visited[pointed].test(node))
            return false;
        visited[pointed].set(node);

        if(node == c.v2)
            return true;

        if(c.condition.test(node)) {
            if(pointed && pass_collider(node, c))
                return true;
        } else if(pass_noncollider(pointed, node, c))
            return true;

        return false;
    }

    bool has_active_trail(const Constraint& c) {
        visited[0] = 0;
        visited[1] = 0;
        
        for(uchar neighbour = 0; neighbour < N; neighbour++) {
            if((EDGE(c.v1, neighbour) || BIEDGE(c.v1, neighbour)) && handle(true, neighbour, c))
                return true;
            
            if(EDGE(neighbour, c.v1) && enter_by_outward_edge(neighbour, c.v1, c))
                return true;
        }
        return false;
    }

    template<bool Directed = false>
    bool can_reach(uchar node, uchar target, Bits& visited, Bits intervent = {}) const {
        if(node == target)     return true;
        if(visited.test(node)) return false;
        visited.set(node);

        for(uchar v = 0; v < N; v++) {
            if(((edge(node, v) && !intervent.test(v)) ||
                (!Directed && edge(v, node) && !intervent.test(node) ) ||
                (!Directed && biedge(v, node) && !intervent.test(v) && !intervent.test(node))) &&
                    can_reach<Directed>(v, target, visited, intervent))
                return true;
        }
        return false;
    }

    bool has_walk_between(uchar node, uchar target, Bits intervent = {}) const {
        Bits visited;
        return can_reach<false>(node, target, visited, intervent);
    }

    bool has_directed_path(uchar node, uchar target, Bits intervent = {}) const {
        //if(ComplType == MINIMAL)
        //    return solution.has_directed_path(node, target);
        Bits visited;
        return can_reach<true>(node, target, visited, intervent);
    }

    template<bool Nontrivial>
    bool has_inducing_trail(uchar v1, uchar v2) {
        visited[1].set(v1);

        for(uchar neighbour = 0; neighbour < N; neighbour++) {
            if(Nontrivial && neighbour == v2)
                continue;
            if(edge(v1, neighbour) && handle_inducing_trail(neighbour, v2))
                return true;
        }
        return false;
    }

    bool handle_inducing_trail(uchar v1, uchar v2) {
        if(v1 == v2) return true;
        if(visited[1].test(v1)) return false;

        visited[1].set(v1);

        for(uchar neighbour = 0; neighbour < N; neighbour++)
            if(edge(v1, neighbour) && biedge(v1, neighbour) &&
                handle_inducing_trail(neighbour, v2))
                    return true;
        return false;
    }
    
private:
    const Completion& completion;
    Bits visited[2];
};

GraphTools::GraphTools(const Completion& completion):
    completion(completion) {}

#define SEPARATION_QUERY(ACTION) \
    if(completion.requires(SigmaSeparation))\
        return HelperClass<SIGMA_SEPARATION>(completion).ACTION;\
    return HelperClass<D_SEPARATION>(completion).ACTION;

bool GraphTools::has_nontrivial_inducing_trail(uchar x, uchar y) {
    SEPARATION_QUERY( has_inducing_trail<true>(x, y) )
}

bool GraphTools::has_inducing_trail(uchar x, uchar y) {
    SEPARATION_QUERY( has_inducing_trail<false>(x, y) )
}

bool GraphTools::has_active_trail(const Constraint& c) {
    SEPARATION_QUERY( has_active_trail(c) );
}

bool GraphTools::has_walk_between(uchar x, uchar y) const {
    return HelperClass<>(completion).has_walk_between(x, y);
}

bool GraphTools::has_directed_path(uchar x, uchar y) const {
    return HelperClass<>(completion).has_directed_path(x, y);
}

#pragma once

#include <ilcplex/ilocplex.h>
#include <Instance.hpp>
#include <unordered_map>
#include <unordered_set>
#include <functional>

struct ObjectiveFunction;

class CPLEX {
public:
    enum { Sat, Unsat, Undefined };
    
    CPLEX(const Instance&, uchar);
    
    void add_item(const Constraint&);
    void add_core(const std::list<Constraint>&);
    void add_intervention_constraint(const Constraint&, const Constraint&);
    
    Weight lowerbound();
    
    void set(const Constraint&, bool);
    void reset(const Constraint&);
    
    void finalize_expression();
    void finalize_constraints();
    
    void update_reduced_cost_fixing(Weight);
    
    inline void open_scope() {
        scopes.push_back( std::vector<const Constraint*>() );
    }
    
    inline bool induced_independence(uchar x, uchar y) const {
        return y < x? induced_independence(y, x):
            unsat_deps[x][y] > 0;
    }
    
    inline bool can_induce_independence(const IloNumVar& var, const Constraint& c) const {
        return (var.getLB() == 1 && !c.independence) ||
            (var.getUB() == 0 && c.independence);
    }
    
    inline size_t time_spent_solving() const { return total_solve_times; }
    
    void close_scope();
    void fix(const Constraint*, bool);
    
    void ensure_correctness(const std::function<void(void)>&) const;

private:
    IloEnv env;
    
    const Instance& instance;
    IloModel model;
    IloNumVarArray vars;
    IloCplex cplex;
    IloObjective objective;
    IloRangeArray constraints;
    std::vector<std::vector<size_t> > unsat_deps;
    size_t total_solve_times;
    IloNumExpr expression;

    std::unordered_map<Constraint::Id, size_t> fixing_depth;
    
    std::unordered_map<Constraint::Id, IloNumVar> var_lookup;
    std::unordered_map<Constraint::Id, IloRange> constr_lookup;
    
    std::vector<Constraint::Id> var_to_constr;
    std::vector< std::vector<const Constraint*> > scopes;
};
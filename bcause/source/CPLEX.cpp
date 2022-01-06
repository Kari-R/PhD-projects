#include <CPLEX.hpp>
#include <GraphEvaluator.hpp>
#include <debug.hpp>
#include <cmath>

#define EPS 1e-7

CPLEX::CPLEX(const Instance& instance, uchar threads):
    instance(instance), model(env), vars(env), cplex(model),
    objective(IloMinimize(env, IloExpr(env))),
    constraints(env),
    unsat_deps(instance.size(),
        std::vector<size_t>(instance.size(), 0)),
    total_solve_times(0)
{
    cplex.setOut(env.getNullStream());
    model.add(objective);
    
    cplex.setParam(IloCplex::SimDisplay, 0);
    cplex.setParam(IloCplex::MIPDisplay, 0);
    cplex.setParam(IloCplex::Threads, threads);
    
    expression = objective.getExpr();
    
    open_scope();
}

void CPLEX::finalize_expression() {
    model.add(vars);
    objective.setExpr(expression);
}

void CPLEX::finalize_constraints() {
    model.add(constraints);
}

void list_to_str(std::stringstream& stream, Bits bits) {
    for(size_t i = 0; i < bits.size(); i++)
        if(bits.test(i))
            stream << i;
}

void CPLEX::add_item(const Constraint& c) {
    ensure(var_lookup.find(c.ident()) == var_lookup.end());
    
    IloNumVar var(env, 0, 1, ILOFLOAT);
    vars.add(var);
    
    var_lookup.insert(std::make_pair(c.ident(), var));
    expression += c.weight * var;
    var_to_constr.push_back(c.ident());
}

void CPLEX::add_core(const std::list<Constraint>& core) {
    IloExpr expr(env);
    for(const Constraint& c: core) {
        ensure(var_lookup.find(c.ident()) != var_lookup.end());
        expr += var_lookup[c.ident()];
    }
    constraints.add(IloRange(expr >= 1));
}

void CPLEX::add_intervention_constraint(const Constraint& c1, const Constraint& c2) {
    ensure((c1.intervent & c2.intervent) == c1.intervent);
    ensure(c1.v1 == c2.v1 && c1.v2 == c2.v2);
    ensure(c1.condition == c2.condition && c1.intervent != c2.intervent);
    ensure(var_lookup.find(c1.ident()) != var_lookup.end());
    ensure(var_lookup.find(c2.ident()) != var_lookup.end());

    #define ADD_CAUSE_EFFECT(cause, effect) \
	{\
	    IloExpr expr(env);\
	    expr += (effect) - (cause);\
	    constraints.add(IloRange(expr <= 0));\
	}\

    IloNumVar v1 = var_lookup[c1.ident()];
    IloNumVar v2 = var_lookup[c2.ident()];

    if(c2.independence) {
        if(c1.independence)
            ADD_CAUSE_EFFECT(v1, v2)      // IND(a) -> IND(ab)
        else
            ADD_CAUSE_EFFECT(1-v1, v2)    // !DEP(a) -> IND(ab)
    } else {
        if(c1.independence)
            ADD_CAUSE_EFFECT(v2, 1-v1)    // DEP(ab) => !IND(a)
        else
            ADD_CAUSE_EFFECT(v2, v1)      // DEP(ab) => DEP(a)
    }
}

Weight CPLEX::lowerbound() {
    size_t start_time = get_time();
    if(!cplex.solve()) {
       if(instance[RCFixing])
           return INF;
       
       print("Error: No CPLEX solution!\n"
           "  The LP model is printed in 'error.lp'\n");
       cplex.exportModel("error.lp");
       std::exit(-1);
    }
    total_solve_times += get_time() - start_time;
    return lround(ceil(cplex.getObjValue() - EPS));
}

void CPLEX::set(const Constraint& c, bool unsat) {    
    ensure(var_lookup.find(c.ident()) != var_lookup.end());

    IloNumVar& var = var_lookup.find(c.ident())->second;
 
    var.setLB(unsat);
    var.setUB(unsat);
}

void CPLEX::reset(const Constraint& c) {
    auto it = fixing_depth.find(c.ident());
    if(it != fixing_depth.end()) {
        if(it->second < scopes.size())
            return;
        fixing_depth.erase(it);
    }
    
    IloNumVar& var = var_lookup.find(c.ident())->second;
    var.setLB(0);
    var.setUB(1);
}

void CPLEX::update_reduced_cost_fixing(Weight ub) {

    Weight lb = cplex.getObjValue();
    if(abs(lb - ub) < EPS) return;

    IloNumArray red_costs(env);
    cplex.getReducedCosts(red_costs, vars);

    IloNumArray values(env);
    cplex.getValues(values, vars);
    
    for(size_t i = 0; i < (size_t)vars.getSize(); i++) {
        if(vars[i].getLB() == vars[i].getUB())
            continue;
        
        Constraint::Id id = var_to_constr[i];
        const Constraint& constr = instance.get(id);
        
        if(IloAbs(values[i]) < EPS) {
            if(red_costs[i] <= EPS) continue;
            if(lb + red_costs[i] <= ub) continue;
            fix(&constr, CPLEX::Sat);
        } else {
            if(red_costs[i] >= -EPS) continue;
            if(lb - red_costs[i] <= ub) continue;
            fix(&constr, CPLEX::Unsat);
        }
    }
}

void CPLEX::fix(const Constraint* c, bool value) {
    IloNumVar& var = var_lookup.find(c->ident())->second;
    if(var.getLB() != 0 || var.getUB() != 1)
        return;
    
    set(*c, value);
    scopes.back().push_back(c);
    fixing_depth[c->ident()] = scopes.size();
}

void CPLEX::close_scope() {
    ensure(!scopes.empty());
    
    auto& scope = scopes.back();
    while(!scope.empty()) {
        const Constraint* c = scope.back();

        reset(*c);        
        scope.pop_back();
    }
    scopes.pop_back();
}

void CPLEX::ensure_correctness(const std::function<void(void)>& fun) const {
    if(!instance[RCFixing]) {
        fun();
        return;
    }
    
    std::vector<long> orig_lbs, orig_ubs;
    for(size_t i = 0; i < (size_t)vars.getSize(); i++) {
        orig_lbs.push_back(vars[i].getLB());
        orig_ubs.push_back(vars[i].getUB());
    }
    fun();
    
    for(size_t i = 0; i < (size_t)vars.getSize(); i++)
        if(orig_lbs[i] != vars[i].getLB() || orig_ubs[i] != vars[i].getUB()) {
            print("[Error in RC fixing]\n"
                "For (%)\n"
                "  LB: % -> %\n"
                "  UB: % -> %\n",
                    var_to_constr[i],
                    orig_lbs[i], vars[i].getLB(),
                    orig_ubs[i], vars[i].getUB());
            std::exit(0);
        }
}

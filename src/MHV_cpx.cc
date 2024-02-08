#include <vector>
#include <cassert>
#include <chrono>
#include "wcsp.hh"
#include "MHV_cpx.hh"

using namespace std::chrono;
using std::vector;

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

// costs[i][K[j][i]] is the i-th element of j-th vector
// costs[i] is sorted ascendent
MHV_cplex::MHV_cplex(const vector<vector<Cost>>& costs): costs(costs), e(costs.size()){

    model = IloModel(env);

    // variables (h[][])
    // h[i][k]  <=>  h_mhv[i] > costs[k]
    h = vector<IloNumVarArray>(e);
    for (int i = 0; i < e; i++) {
        h[i] = IloNumVarArray(env, costs[i].size(), 0, 1, ILOBOOL);
    }

    // consistency on h
    // ∀i ∀m > 0 : h[i][m] <= h[i][m - 1]
    for (int i = 0; i < e; ++i) {
        model.add(h[i][0] >= 1); // it's always true
        for (int m = 1; m < costs[i].size(); ++m) model.add(h[i][m - 1] - h[i][m] >= 0);
    }

    // objective
    IloExpr obj_expr = IloExpr(env);
    for (int i = 0; i < e; ++i) {
        assert(costs[i][0] == 0);
        for (int m = 1; m < costs[i].size(); ++m) {
            assert(costs[i][m] - costs[i][m - 1] > 0);
            obj_expr += h[i][m]*(costs[i][m] - costs[i][m - 1]);
        }
    }
    IloObjective obj = IloMinimize(env, obj_expr);
    model.add(obj);
    cplex = IloCplex(model); // equivalent to: cplex = IloCplex(env); cplex.extract(model)
    obj_expr.end();

    // configuration of output information given by cplex
    cplex.setOut(env.getNullStream());
    cplex.setWarning(env.getNullStream());
    cplex.setError(env.getNullStream());
}

// h_mhv hits core (i.e. ∃i : h_mhv[i] > costs[core[i]])
void MHV_cplex::addCore(const vector<int>& core) {
    IloEnv env = model.getEnv();
    IloExpr expr(env);
    assert(core.size() == e);
    for (int i = 0; i < e; i++) {
        if (core[i] < costs[i].size() - 1) {  // i-dimension can be hit (different to last finite cost)
            expr += h[i][core[i] + 1];
        }
    }
    IloConstraint ctr = (expr >= 1);
    model.add(ctr);
    expr.end(); // clear memory [expr is passed by value to .add]
}

bool MHV_cplex::solve(vector<Cost>& found_h, vector<int>& found_h_idom, Cost & found_cost) {
    bool feasible = cplex.solve() == IloTrue;
    if (not feasible) return false;

    assert(cplex.getStatus() == IloAlgorithm::Optimal);

    found_h = vector<Cost>(e, 0);
    found_h_idom = vector<int>(e, 0);
    for (int i = 0; i < e; ++i) {
        int m = costs[i].size() - 1;
        while (m > 0 and cplex.getIntValue(h[i][m]) == 0) --m;
        found_h_idom[i] = m;
        found_h[i] = costs[i][m];
    }
    found_cost = sum(found_h);
    return true; //feasible
}

bool MHV_cplex::solve_MHV(long& time) {
    auto start = high_resolution_clock::now();
    bool feasible = solve(best_h, best_h_idom, best_cost);
    assert(feasible);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    time += duration.count();
    return true; //feasible
}

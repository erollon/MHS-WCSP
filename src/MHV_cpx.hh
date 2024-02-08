#ifndef MHV_CPX_HH
#define MHV_CPX_HH

#include <vector>
#include "wcsp.hh"

using std::vector;

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN;

class MHV_cplex {
public:
    MHV_cplex(const vector<vector<Cost>>& costs);
    ~MHV_cplex() {env.end(); }

    void addCore(const vector<int>& core);
    bool solve_MHV(long& time);
    Cost getCost_MHV() { return best_cost; }
    vector<int> getMHV_idom() { return best_h_idom; }

private:
    vector<vector<Cost> > costs;  // costs[i][m] is the m-th cost of i-th function  (i.e. i-th component of each core)
                                  // (costs[i].size() - 1 is the index of the last finite cost)
                                  //       costs[i][0] = 0;
    int e;                        // num dimensions of cores
    IloEnv env;
    IloCplex cplex;
    IloModel model;
    vector<IloNumVarArray> h;     // h variables

    vector<Cost> best_h;          //MHV
    vector<int>  best_h_idom;     //MHV
    Cost         best_cost=0;     //cost MHV

    bool solve(vector<Cost>& found_h, vector<int>& found_h_idom, Cost & found_cost);
};
#endif

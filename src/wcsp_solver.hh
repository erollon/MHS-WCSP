#ifndef WCSP_SOLVER_HH
#define WCSP_SOLVER_HH

#include <vector>

#include "MHV_cpx.hh"
#include "csp.hh"
#include "wcsp.hh"

using std::vector;

class WcspSolver {
public:
  WcspSolver(const Wcsp &wcsp);     // orig ihs
  WcspSolver(const Wcsp &wcsp, const vector<vector<int>>& part);
  WcspSolver(const Wcsp &wcsp, bool generator_type);

  ~WcspSolver() {delete mhvs; delete ces;}
  Cost solve();
  void case_study_abstract_core();

private:
    const Wcsp& wcsp;               // WCSP data
    MHV_cplex* mhvs;                // MHV solver for the set of cores
    vector<vector<int>> nd_cores;   // non-dominated set of cores
    CoreCSP* ces;                   // CSP solver

    void add_core(vector<vector<int>> &K2, const vector<int> &k);
    Cost solve_lb(vector<int>& h, const vector<bool>& active,
                  int& iteration, int& ncores, long& t_solver, long& t_mhv);
    void update_non_dominated(const vector<int>& k);
};

#endif

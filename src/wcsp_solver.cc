#include "wcsp_solver.hh"
#include "csp_sat.hh"


WcspSolver::WcspSolver(const Wcsp &wcsp, const vector<vector<int>>& part)
    : wcsp(wcsp), mhvs(nullptr) {
    ces = new CSP_sat(wcsp, part);
}

WcspSolver::WcspSolver(const Wcsp &wcsp): wcsp(wcsp), mhvs(nullptr) {
    ces = new CSP_sat(wcsp);
}

void WcspSolver::case_study_abstract_core() {
    ces->case_study_abstract_core();
}

void WcspSolver::update_non_dominated(const vector<int>& k) {
    int i = 0;
    while (i < nd_cores.size()) {
        if (nd_cores[i] <= k) // k dominates nd_cores[i]
            nd_cores.erase(nd_cores.begin() + i);
        else
            ++i;
    }
    nd_cores.push_back(k);
}

Cost WcspSolver::solve() {
  int iteration = 0;
  long t_solver = 0;
  long t_mhv = 0;
  int ncores = 0;
  Cost lb = 0;
  Cost ub = wcsp.ub;

  mhvs = new MHV_cplex(ces->part);
  vector<int> h(ces->part.size(), 0);
  assert(h.size() == ces->part.size());

  while (not ces->solve(h, t_solver)) {
    // add cores to the mhv solver
    const vector<vector<int>> &C = ces->getCores();
    for (const vector<int> &k : C) {
      mhvs->addCore(k);
      update_non_dominated(k);
    }
    ncores += C.size();

    // compute new hitting vector
    bool hv_found = mhvs->solve_MHV(t_mhv);
    assert(hv_found);
    h = mhvs->getMHV_idom();

    // update bounds
    lb = ces->vector_cost(h);
    assert(lb == mhvs->getCost_MHV());
    ub = min(ub, wcsp.costAssign(ces->getSolution()));

    iteration++;
    cout << "Iteration " << iteration
         << "  lb " << wcsp.lb + lb
         << "  ub " << wcsp.lb + ub
         << "  cores " << ncores << " non_dom_cores " << nd_cores.size()
         << "  time " << t_solver / 1000000.0 << " " << t_mhv / 1000000.0
         << "  satcalls " << ces->sat_calls << endl;

  }
  cout << "   optimum (subprob): " << lb << endl;
  return wcsp.lb + lb;
}

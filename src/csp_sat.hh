#ifndef CSPSAT_HH
#define CSPSAT_HH

#include "sat-cadical/src/cadical.hpp"
#include "wcsp.hh"
#include "function.hh"
#include "csp.hh"

class CSP_sat : public CoreCSP {
public:
    CSP_sat(const Wcsp& wcsp);
    CSP_sat(const Wcsp& wcsp, const vector<vector<int>>& partitions);
    bool solve(vector<int> h);

private:
    const int NOLIT = -1;
    CaDiCaL::Solver solver;

    int lit_num = 1;                    //next avaliable literal
    vector<int> var2lit;
    vector<int> part2lit;

    int varVal2lit(int var, int val) const;
    int partICost2lit(int func, int idx_cost) const;

    int smallestFail(const vector<int>& h, int func);
    void buildSolution();
    vector<int> build_base_model();     // it builds basic SAT model
    bool solve(const vector<int> &h, vector<int>& k);

    void at_least_one(int s_lit, int e_lit);
    void at_most_one(int s_lit, int e_lit);
    void at_most_one(const vector<int>& literals);

    vector<Cost> compact(int lit_num_f1, const vector<Cost>& costs_f1,
                         int lit_num_f2, const vector<Cost>& costs_f2);

    void add_hard_greater_than();
    void add_hard_alldiff();
    void case_study_abstract_core();
};
#endif

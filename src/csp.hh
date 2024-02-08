#ifndef CORE_CSP_HH
#define CORE_CSP_HH

#include <vector>
#include <chrono>
#include "wcsp.hh"

using namespace std::chrono;

class CoreCSP {
public:
    const Wcsp& wcsp;
    vector<vector<int>> C;      //cores, C ⊆ {k | h ≤ k, wcsp^k unsat}
    vector<int> sol;            //solution, wcsp^h(sol) = true
    vector<vector<Cost>> part;  // costes de las particiones
    int sat_calls;

    CoreCSP(const Wcsp& wcsp) : wcsp(wcsp), sat_calls(0) {}

    virtual void case_study_abstract_core() = 0;

    // if wcsp^h sat   -> return true
    // if wcsp^h unsat -> return false & store a set of cores at C (see getCores())
    virtual bool solve(vector<int> h) = 0;

    // if wcsp^h sat   -> return true
    // if wcsp^h unsat -> return false & store a set of cores at C (see getCores())
    bool solve(const vector<int> &h, long &time) {
        auto start = high_resolution_clock::now();
        bool ret = solve(h);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        time += duration.count();
        return ret;
    }

    // PRE: solve(h) called & returned FALSE
    //      i.e. wcsp^h unsat
    // return a set of cores for wcsp^h
    //      i.e. return C s.t. C ⊆ {k | h ≤ k, wcsp^k unsat}
    const vector<vector<int>>& getCores() {return C;}

    // PRE: solve(h) called & returned TRUE
    //      i.e. wcsp^h sat
    // return a solution for wcsp^h
    //      i.e. return sol s.t. wcsp^h(sol) = true
    const vector<int>& getSolution() {return sol;}

    Cost vector_cost(const vector<int>& h) {
        assert(h.size() == part.size());
        Cost c = 0;
        for (int i = 0; i < h.size(); ++i) {
          c = c + part[i][h[i]];
        }
        return c;
    }

protected:
    // Post: returna índex con mínimo coste que todavía se puede aumentar (es decir
    //        que es diferente de costs[i].size() - 1); -1 en caso de que no haya
    int idx_min_Cost(const vector<int>& k) {
        //assert(wcsp.nfuncs == k.size() and k.size() > 0);
        assert(part.size() == k.size() and k.size() > 0);
        int min_f = -1;
        Cost min_cost = wcsp.ub;
        for (int f = 0; f < part.size(); ++f) {
            assert(k[f] < part[f].size());
            Cost cost = part[f][k[f]];
            if (k[f] < part[f].size() - 1 and cost < min_cost) {
                min_f = f;
                min_cost = cost;
            }
        }
        return min_f;
    }
};

#endif

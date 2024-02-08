#include <set>
#include <iostream>
#include <algorithm>
#include <vector>
#include "csp_sat.hh"
#include "config.hh"

using std::cout;
using std::set;
using std::find;
using std::vector;

// Post: e_lit not included
//          ∀x: make clause (x_{s_lit} v x_{s_lit + 1} v ... v x_{e_lit - 1})
void CSP_sat::at_least_one(int s_lit, int e_lit) {
    for (int i = s_lit; i < e_lit; ++i) solver.add(i);
    solver.add(0);
}

// Post: e_lit not included
//         ∀x,a,b \in [s_lit, e_lit): make clause -(xa ^ xb) i.e. (-xa v -xb)
void CSP_sat::at_most_one(int s_lit, int e_lit) {
    for (int i = s_lit; i < e_lit; ++i) {
        for (int j = i + 1; j < e_lit; ++j) {
            solver.add(-i);
            solver.add(-j);
            solver.add(0); //cerr << endl;
        }
    }
}

// at most one over all literals
void CSP_sat::at_most_one(const vector<int>& literals) {
    int n = literals.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            solver.add(-literals[i]);
            solver.add(-literals[j]);
            solver.add(0);
        }
    }
}

vector<int> CSP_sat::build_base_model() {
    lit_num = 1;

    var2lit = vector<int>(wcsp.nvars);
    for (int x = 0; x < wcsp.nvars; x++) {
        // ∀x,a: make atom xa
        //  where x variable, a in domain(x)
        var2lit[x] = lit_num;
        at_least_one(lit_num, lit_num + wcsp.domsize[x]);
        at_most_one(lit_num, lit_num + wcsp.domsize[x]);
        lit_num += wcsp.domsize[x];
    }

    vector<int> func2lit = vector<int>(wcsp.nfuncs);
    for (int f = 0; f < wcsp.nfuncs; f++) {
        // ∀f,c: make atom f_c
        //  where f function, c is cost of f, c > 0
        func2lit[f] = lit_num;
        lit_num += wcsp.costs[f].size();

        // ∀f,t: make clause (-x1_a1 v -x2_a2 v ... v -xn_an v <block>),
        //  where t is tuple <a1, a2, ..., an>(cost c) of function f(x1, x2, ..., xn)
        //      - if c == 0   : <block> = true   i.e. clause not included
        //      - if c == top : <block> = false  i.e. hard clause (<block> not included)
        //      - else        : <block> = f_c    i.e. soft clause (<block> assumption)
        const Function& func = wcsp.functions[f];
        vector<int> scope = func.getScope();
        for (int t = 0; t < func.numTuples(); t++) {
            Cost cost = func.getCost(t);
            if (cost > 0) {
                if (cost < wcsp.ub) {
                    int literal = func2lit[f] + wcsp.cost2index(f, cost);
                    solver.add(literal);
                }
                vector<int> tuple = func.getTuple(t);
                assert(scope.size() == tuple.size());
                for (int xf = 0; xf < func.arity(); xf++) {
                    int x = scope[xf];
                    int a = tuple[xf];
                    solver.add(-varVal2lit(x,a));
                }
                solver.add(0);
            }
        }
    }
    return func2lit;
}

vector<Cost> CSP_sat::compact(int lit_num_f1, const vector<Cost>& costs_f1,
                              int lit_num_f2, const vector<Cost>& costs_f2) {
    set<Cost> aux = {0};
    for (int idx_a = 0; idx_a < costs_f1.size(); ++idx_a) {
      Cost a = costs_f1[idx_a];
      for (int idx_b = 0; idx_b < costs_f2.size(); ++idx_b) {
          Cost b = costs_f2[idx_b];
          if (a + b < wcsp.ub) aux.insert(a + b);
      }
    }
    vector<Cost> sum_costs(aux.begin(), aux.end());

  // generalized totalizer
  for (int idx_a = 1; idx_a < costs_f1.size(); ++idx_a) {  // w + 0 --> w
      int idx_c = std::find(sum_costs.begin(), sum_costs.end(), costs_f1[idx_a]) - sum_costs.begin();
      solver.add(-(lit_num_f1 + idx_a));
      solver.add(lit_num + idx_c);
      solver.add(0);
  }
  for (int idx_b = 1; idx_b < costs_f2.size(); ++idx_b) { // 0 + w --> w
      int idx_c = std::find(sum_costs.begin(), sum_costs.end(), costs_f2[idx_b]) - sum_costs.begin();
      solver.add(-(lit_num_f2 + idx_b));
      solver.add(lit_num + idx_c);
      solver.add(0);
  }
  for (int idx_a = 1; idx_a < costs_f1.size(); ++idx_a) {  // w1 + w2 --> w
    for (int idx_b = 1; idx_b < costs_f2.size(); ++idx_b) {
        Cost t = costs_f1[idx_a] + costs_f2[idx_b];
        //assert(t > 0);
        if (t < wcsp.ub) {
            int idx_c = std::find(sum_costs.begin(), sum_costs.end(), t) - sum_costs.begin();
            solver.add(lit_num + idx_c);
        }
        solver.add(-(lit_num_f1 + idx_a));
        solver.add(-(lit_num_f2 + idx_b));
        solver.add(0);
    }
  }

  return sum_costs;
}

// Pre: alldiff or greaterthan
void CSP_sat::case_study_abstract_core() {
    cout << "abstract cores case study" << endl;
    assert(wcsp.alldiff or wcsp.greaterthan);
    assert(wcsp.nvars == wcsp.functions.size());
    assert(part.size() == wcsp.nvars);

    const int N = wcsp.nvars;
    vector<int> part2lit_new(N - 1);

    for (int a = 1; a < N; ++a) {  // for all costs / domain values

        int act_lit_num = part2lit[0] + a - 1; // part2lit tiene que ser equivalente a func2lit de build_base_model
        vector<Cost> act_costs = {0, a};

        for (int i = 1; i < N; ++i) {  // sum cost "a" over all functions
            solver.add(-(part2lit[i] + a));
            solver.add(lit_num + 1);
            solver.add(0);    //0 + a --> a
            for (int j = 1; j < act_costs.size(); ++j) { // w + 0 --> w
                solver.add(-(act_lit_num + j));
                solver.add(lit_num + j);
                solver.add(0);
            }
            for (int j = 1; j < act_costs.size(); ++j) { // w + a --> w + a
                solver.add(-(act_lit_num + j));
                solver.add(-(part2lit[i] + a));
                solver.add(lit_num + j + 1);
                solver.add(0);
            }
            act_costs.push_back(i*a + a); // i*a is the greatest cost in act_costs
            act_lit_num = lit_num;
            lit_num = lit_num + act_costs.size();
        }
        assert(act_lit_num + act_costs.size() == lit_num);
        part[a - 1] = act_costs;
        part2lit_new[a - 1] = act_lit_num;
    }
    part.pop_back();
    part2lit = part2lit_new;
}

void CSP_sat::add_hard_alldiff() { // pairwise encoding
    // equal_one over all domain values of each variable already included in the model

    const int N = wcsp.nvars;
    assert(wcsp.nvars > 0 and wcsp.nvars == wcsp.domsize[0]);
    // at_most_one over each domain value across variables
    for (int a = 0; a < N; ++a) {
        vector<int> literals(N);
        for (int i = 0; i < wcsp.nfuncs; ++i) literals[i] = varVal2lit(i, a);
        at_most_one(literals);
    }
}

void CSP_sat::add_hard_greater_than() {
    const int N = wcsp.nvars;
    if (N >= wcsp.ub) {
        cout << "Error: hard greater than" << endl;
        exit(0);
    }

    assert(wcsp.nvars == wcsp.nfuncs);
    vector<int> g2lit = vector<int>(wcsp.nfuncs); // funciones g_i(x_i) = x_i auxiliares
    for (int i = 0; i < N; ++i) {
        g2lit[i] = lit_num;
        for (int a = 0; a < N; ++a) {
            solver.add(lit_num + a);
            solver.add(-varVal2lit(i, a));
            solver.add(0);
        }
        lit_num += N;
    }

    int act_lit_num = g2lit[0];
    for (int id_f = 1; id_f < wcsp.functions.size(); ++id_f) {
        // actualizar sumatorio hasta g_{id_f}
        for (int idx_a = 0; idx_a < N; ++idx_a) {  // w1 + w2 --> w
          for (int idx_b = 0; idx_b < N; ++idx_b) {
              Cost t = idx_a + idx_b;
              if (t < N) {
                  solver.add(lit_num + t);
                  solver.add(-(act_lit_num + idx_a));
                  solver.add(-(g2lit[id_f] + idx_b));
                  solver.add(0);
              }
          }
        }
        act_lit_num = lit_num;
        lit_num = lit_num + N;
    }


    for (int i = 0; i < N; ++i) {
        solver.add(-(act_lit_num + i));
        solver.add(0);
    }
}

CSP_sat::CSP_sat(const Wcsp& wcsp, const vector<vector<int>>& partitions) : CoreCSP(wcsp) { // Partitioning ihs

    vector<int> func2lit = build_base_model();
    if (wcsp.greaterthan) add_hard_greater_than();
    else if (wcsp.alldiff) add_hard_alldiff();

    part2lit.reserve(partitions.size());
    part.reserve(partitions.size());
    for (int i = 0; i < partitions.size(); ++i) {
        assert(partitions[i].size() != 0);
        int id_f = partitions[i][0];
        if (partitions[i].size() == 1) {
            if (wcsp.costs[id_f].size() > 1) {  // soft
                part.push_back(wcsp.costs[id_f]);
                part2lit.push_back(func2lit[id_f]);
            }
        } else {
            int act_lit_num = func2lit[id_f];
            vector<Cost> act_costs(wcsp.costs[id_f].begin(), wcsp.costs[id_f].end());
            for (int j = 1; j < partitions[i].size(); ++j) {
                // restricciones sobre las dos funciones
                int new_f = partitions[i][j];
                act_costs = compact(act_lit_num, act_costs, func2lit[new_f], wcsp.costs[new_f]);
                act_lit_num = lit_num;
                lit_num = lit_num + act_costs.size();
            }
            if (act_costs.size() > 1) { // soft
                part2lit.push_back(act_lit_num);
                part.push_back(act_costs);
            }
            assert(act_lit_num + act_costs.size() == lit_num);
        }
    }
}

CSP_sat::CSP_sat(const Wcsp& wcsp) : CoreCSP(wcsp) { // orig ihs
    vector<int> f2lit = build_base_model();
    assert(f2lit.size() == wcsp.costs.size());

    if (wcsp.greaterthan) add_hard_greater_than();
    else if (wcsp.alldiff) add_hard_alldiff();

    for (int i = 0; i < wcsp.costs.size(); ++i) {
        if (wcsp.costs[i].size() > 1) { // soft
            part2lit.push_back(f2lit[i]);
            part.push_back(wcsp.costs[i]);
        }
    }

    assert(part2lit.size() == part.size());
}

void CSP_sat::buildSolution() {
    sol = vector<int>(wcsp.nvars);
    for (int x = 0; x < wcsp.nvars; ++x) {
        int count = 0;
        for (int a = 0; a < wcsp.domsize[x]; ++a) {
            if (solver.val(varVal2lit(x,a)) > 0) {
                sol[x] = a;
                count++;
            }
        }
        assert(count == 1);
    }
}


// solve wcsp^h
// if sat   : return true  ,  sol is a solution,   i.e. wcsp^h(sol)=true
// if unsat : return false ,  C is a set of cores, i.e. C ⊆ {k | h ≤ k, wcsp^k unsat}
bool CSP_sat::solve(vector<int> h) {
    assert(h.size() == part.size());

    C = vector<vector<int>>(0);
    vector<int> k;
    while (not solve(h, k)) {
        if (HsConfig::hsOption == HS_MIN) k = h;
        else if (HsConfig::hsOption == HS_GREEDY) { // HS-wcsp_greedy:
            // improve core k
            vector<int> h_;
            do {
                h_ = k;
                assert(h <= k);
                int i = idx_min_Cost(h_);
                assert(i != -1); // it must be a core
                ++h_[i];
            } while (not solve(h_, k));
        }
        else if (HsConfig::hsOption == HS_MAX) { // HS-WCSP_max:
            vector<int> k_idx = k;
            int i;
            while ((i = idx_min_Cost(k_idx)) != -1) {
                vector<int> h_ = k;
                assert(h <= k);
                ++h_[i];
                if (solve(h_, k)) k_idx[i] = part[i].size() - 1;
                else {
                    for (int i = 0; i < k.size(); ++i)
                        k_idx[i] = k_idx[i] == part[i].size() - 1 ? k_idx[i] : k[i];
                }
            }
        }

        C.push_back(k);
        for (int i = 0; i < h.size(); ++i) if (k[i] < part[i].size() - 1) {
            h[i] = part[i].size() - 1;
        }
    }

    buildSolution(); // optimal solution or ub
    return C.empty();
}

// solve wcsp^h
// if sat   : return true and k is unchanged
// if unsat : return false; k is a core (i.e. wcsp^k unsat, h ≤ k)
bool CSP_sat::solve(const vector<int> &h, vector<int>& k) {
    ++sat_calls;
    assert(h.size() == part.size());
    for (int f = 0; f < part.size(); ++f) {
        int num_costs = part[f].size();
        for (int c = 1; c < num_costs; ++c) {
            int sign = (c <= h[f]) ? 1 : -1;
            solver.assume(sign*partICost2lit(f, c));
        }
    }
    int r = solver.solve();
    assert(r == 10 or r == 20);
    bool sat = (r == 10);
    if (not sat) {
        k = vector<int>(part.size());
        for (int f = 0; f < k.size(); ++f) k[f] = smallestFail(h, f);
        assert(h <= k);
    }
    return sat;
}

int CSP_sat::varVal2lit(int var, int val) const {
    assert(0 <= val and val < wcsp.domsize[var]);
    return var2lit[var] + val;
}

int CSP_sat::partICost2lit(int func, int idx_cost) const {
    assert(0 <= idx_cost and idx_cost < part[func].size());
    return part2lit[func] + idx_cost;
}

// PRE: solve(h) called & returned FALSE
// POST: given function index f, return cost index c s.t.:
//      c>=h[f], ∃k: wcsp^k unsat, k[f]=c
int CSP_sat::smallestFail(const vector<int>& h, int f) {
    assert(h.size() == part.size());
    int num_costs = part[f].size();
    for (int c = h[f] + 1; c < num_costs; c++) {
        if (solver.failed(-partICost2lit(f,c))) return c - 1;
    }
    return num_costs - 1;
}

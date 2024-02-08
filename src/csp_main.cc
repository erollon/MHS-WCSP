#include "csp_sat.hh"
#include "utils.cc"
#include <iostream>
#include <set>
using namespace std;
using std::cerr;

vector<Cost> cost_of(const vector<int> &idx, const Wcsp &wcsp) {
  vector<Cost> r(wcsp.nfuncs);
  for (int f = 0; f < wcsp.nfuncs; f++) {
    assert(idx[f] <= wcsp.costs[f].size());
    if (idx[f] == wcsp.costs[f].size())
      r[f] = wcsp.ub;
    else
      r[f] = wcsp.costs[f][idx[f]];
  }
  return r;
}

vector<int> fill(vector<int> v1, vector<int> v2) {
  for (int i = 0; i < v2.size(); i++)
    v1[i] = v2[i];
  return v1;
}

int main(int argc, char const *argv[]) {
  Wcsp wcsp;
  wcsp.read(argv[1]);

  vector<int> fs, cs;
  long num_tup = 1;
  long MAX_NUM_TUP = 1000;
  for (int f = 0; f < wcsp.nfuncs; f++) {
    int c = wcsp.costs[f].size();
    num_tup *= c;
    if (num_tup > MAX_NUM_TUP)
      break;
    fs.push_back(f);
    cs.push_back(c);
  }
  Function enum_h(fs, cs, 0, 1);
  // enum_h.kk();

  CoreCSP *fc = new ForwardChecking(wcsp);
  CoreCSP *sat = new CSP_sat(wcsp);

  cerr << "costs:" << endl;
  for (const auto &c : wcsp.costs)
    cerr << "\t" << c << endl;

  cerr << "numTuples = " << enum_h.numTuples() << endl;
  vector<int> h0(wcsp.nfuncs, 0);
  for (int i = 0; i < enum_h.numTuples(); i++) {
    vector<int> h = enum_h.getTuple(i); // hitting vector
    h = fill(h0, h);
    cerr << "h = " << h << " : " << endl;

    for (string name : {"sat", " fc"}) {
      cerr << "  " << name << ": ";
      CoreCSP *csp;
      if (name == "sat")
        csp = sat;
      else
        csp = fc;

      bool b = csp->solve(h);
      if (b) {
        cerr << "  sat, ";
        cerr << "sol = " << csp->getSolution();
      } else {
        cerr << "unsat, ";
        cerr << "C = " << csp->getCores();
        cerr << " (h " << (csp->getCores()[0] == h ? "==" : "!=") << " C[0])";

        /*for (int f = 0; f < wcsp.nfuncs; f++) {
            cerr << "\t";
            for (int c = 0; c < wcsp.costs[f].size(); c++) {
                cerr << "-" << csp.solver.failed(-csp.funcICost2lit(f, c)) << "
        ";
            }
            cerr << endl;
        } */
      }
      cerr << endl;
    }
  }
  ////cerr << "----" << endl << "time = " << time << endl;
}

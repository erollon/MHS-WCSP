#ifndef WCSP_HH
#define WCSP_HH

#include <string>
#include <vector>
#include "function.hh"

using std::string;
using std::vector;


class Wcsp {
public:
  int nvars;
  int nfuncs;
  Cost lb;
  Cost ub;
  vector<int> domsize;
  vector<Function> functions; // explicit in tables

  bool alldiff = false;
  bool greaterthan = false; // sum x_i >= nvars


  // for each variable, functions where it appears
  vector<vector<int>> var2functions;

  vector<vector<Cost>> costs; // functions costs
  vector<int> varOrd;         // variable ordering


public:
  Wcsp();
  // it creates case study instances with only vars, domains and soft unaries
  void create_case_study(int gen_n, int gen_type);
  vector<vector<int>> partition_abstract_core(const vector<vector<int>>& part);
  int getVar(int i) const { return varOrd[i]; }
  // shows the hiteable components of the core
  void showCore(const vector<int> &k) const;
  // computes the position of c in costs[func]
  int cost2index(int func, Cost c) const;
  // computes funcs's idx-th cost (last cost is ub)
  Cost index2cost(int func, int idx) const;
  void sortVariables(int option = 0);
  void read(string fileName);
  void show(int level) const;
  Cost costAssign(const vector<int>& assign) const;
};
#endif

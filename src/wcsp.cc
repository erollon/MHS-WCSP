#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "function.hh"
#include "wcsp.hh"

using std::cout;
using std::endl;
using std::string;
using std::vector;

Wcsp::Wcsp() : nvars(0), nfuncs(0), lb(0), ub(0) {}

void Wcsp::create_case_study(int n, int type) {
    nvars = n;
    lb = 0;
    ub = n*n + 1;
    domsize = vector<int>(nvars, n);
    functions.reserve(nvars);   // nvars unarias
    nfuncs = n;
    var2functions = vector<vector<int>>(nvars);
    costs = vector<vector<Cost>>(nvars);
    for (int i = 0; i < nvars; ++i) {
        var2functions[i] = {i};
        functions.push_back(Function({i}, {n}, 0, ub));
        costs[i].reserve(n);
        for (int c = 0; c < n; ++c) {
            costs[i].push_back(c);
            functions[i].addCost({c}, c);
        }
    }
    alldiff = type == 0;
    greaterthan = type == 1;
}

vector<vector<int>> Wcsp::partition_abstract_core(const vector<vector<int>>& partitions) {
    int n_parts = 0;
    vector<vector<int>> part_core;
    for (const vector<int>& cluster : partitions) {
        set<Cost> costs_cl;
        map<Cost, vector<int>> dict;  // <cost, lista de id funcion con ese coste>
        for (int id_f : cluster) {
            if (costs[id_f].size() == 1) part_core.push_back({id_f}); // hard
            else {  // soft
                costs_cl.insert(costs[id_f].begin(), costs[id_f].end());
                dict[costs[id_f][1]].push_back(id_f);
                for (int i = 2; i < costs[id_f].size(); ++i) {
                    Cost c = costs[id_f][i];
                    Function f_aux = functions[id_f].split(c);
                    int id_f_aux = functions.size();
                    functions.push_back(f_aux);
                    costs.push_back({0, c});
                    assert(functions.size() == costs.size());
                    //actualizar var2functions
                    const vector<int>& scope = f_aux.getScope();
                    for (int var : scope) var2functions[var].push_back(id_f_aux);

                    dict[c].push_back(id_f_aux);
                }
                costs[id_f].erase(costs[id_f].begin() + 2, costs[id_f].end());
            }
        }
        n_parts = n_parts + dict.size();
        for (auto it = dict.begin(); it != dict.end(); ++it) part_core.push_back(it->second);
    }
    cout << "abstract cores: " << n_parts << endl;
    nfuncs = functions.size();
    return part_core;
}

Cost Wcsp::costAssign(const vector<int>& assign) const {
    Cost c = 0;
    for (const Function& f : functions) {
        Cost aux = f.getCostAssg(assign);
        if (aux < ub) c = c + aux;
        else return ub;
    }
    assert(c < ub);
    return c;
}

void Wcsp::showCore(const vector<int> &k) const {
  assert(k.size() == functions.size());
  for (int i = 0; i < k.size(); i++)
    if (k[i] < costs[i].size()) {
      cout << "(";
      functions[i].show(1);
      cout << "cost " << k[i] << ") ";
    }
  cout << endl;
}

int Wcsp::cost2index(int func, Cost c) const {
  for (int i = 0; i < costs[func].size(); i++)
    if (c == costs[func][i])
      return i;
  return costs[func].size();
}

Cost Wcsp::index2cost(int func, int idx) const {
  if (idx >= costs[func].size())
    return ub;
  return costs[func][idx];
}

void Wcsp::sortVariables(int option) {
  vector<pair<int, int>> v(nvars);
  for (int i = 0; i < nvars; i++)
    v[i] = make_pair(var2functions[i].size(), i);
  sort(v.begin(), v.end());
  for (int i = 0; i < nvars; i++)
    varOrd[i] = v[nvars - 1 - i].second; //v[i].second;
}

void Wcsp::read(string fileName) {
  fstream file(fileName);
  if (not file.is_open()) {
    cerr << "Error: File " << fileName << "cannot be opened" << endl;
    exit(EXIT_FAILURE);
  }
  string name;
  int maxdomsize;
  file >> name >> nvars >> maxdomsize >> nfuncs >> ub;
  lb = 0;
  Cost lb2 = 0;
  domsize = vector<int>(nvars);
  for (int i = 0; i < nvars; ++i) file >> domsize[i];
  bool consistent = true; // there must be a zero cost in every cost function
  int unsorted = 0;
  var2functions = vector<vector<int>>(nvars);
  for (int i = 0; i < nfuncs; ++i) {
    int arity;
    file >> arity;
    if (arity == 0) {
      Cost c;
      int ntuples;
      file >> c >> ntuples;
      lb += c;
      assert(ntuples == 0);
    } else {
      vector<int> scope;
      vector<int> domscope;
      for (int k = 0; k < arity; k++) {
        int v;
        file >> v;
        scope.push_back(v);
        domscope.push_back(domsize[v]);
      }
      Cost defcost;
      file >> defcost;
      if (defcost != -1) {  // explicit function
          for (int v : scope) var2functions[v].push_back(functions.size());

          int ntuples;
          file >> ntuples;
          Function f(scope, domscope, defcost, ub);
          for (int j = 0; j < ntuples; ++j) {
            vector<int> t(arity);
            for (int k = 0; k < arity; k++) file >> t[k];
            Cost c;
            file >> c;
            f.addCost(t, c);
          }
          Cost mincost = f.getMinCost();
          if (mincost > 0) { // it is not NC*
            f.substractCost(mincost);
            consistent = false;
            lb2 = lb2 + mincost;
          }
          if (f.sortedScope()) functions.push_back(f);
          else {
            unsorted++;
            Function f2 = f.sortScope();
            functions.push_back(f2);
          }
      }
      else {
          // global constraint ... not considered here
          cerr << "Error: File " << fileName << " has global constraints" << endl;
          exit(EXIT_FAILURE);
      }
    }
  }

  nfuncs = functions.size();

  file.close();

  // final adjustments
  cout << "lb " << lb << " ub " << ub << " before adjustment" << endl;
  if (not consistent)
    cout << "Warning, the problem was not even NC " << lb2 << endl;
  if (unsorted > 0)
    cout << unsorted << " unsorted cost functions" << endl;
  lb = lb + lb2;
  assert(lb < ub);
  ub = ub - lb;
  cout << "lb " << 0 << " ub " << ub << " after adjustment" << endl;
  if (lb > 0)
    for (int i = 0; i < functions.size(); i++)
      functions[i].updateTop(ub);

  // initialize variable ordering, by default lexicographic
  varOrd = vector<int>(nvars);
  for (int i = 0; i < nvars; i++)
    varOrd[i] = i;

  // create cost vectors
  costs = vector<vector<Cost>>(functions.size());
  for (int i = 0; i < functions.size(); ++i)
    costs[i] = functions[i].allCosts();
  cout << "costs ready" << endl;

  Cost new_ub = 1;
  for (int i = 0; i < functions.size(); ++i)
    new_ub += costs[i][costs[i].size() - 1];

  if (new_ub < ub) {
    ub = new_ub;
    for (int i = 0; i < functions.size(); i++) functions[i].updateTop(ub);
    cout << "ub " << ub << " after 2nd adjustment" << endl;
  }

  for (int i = 0; i < nfuncs; i++) {
    assert(functions[i].getTop() == ub);
    assert(functions[i].check());
  }
}

void Wcsp::show(int level) const {
  cout << nvars << " variables ";
  cout << nfuncs << " functions " << lb << " lb, " << ub << " ub" << endl;
  if (level > 0) {
    cout << "Domsize:";
    for (int i = 0; i < domsize.size(); i++)
      cout << domsize[i] << " ";
    cout << endl;

    int maxarity = 0;
    for (int i = 0; i < nfuncs; i++)
      if (functions[i].arity() > maxarity)
        maxarity = functions[i].arity();
    vector<int> frecArities(maxarity + 1, 0);
    for (int i = 0; i < nfuncs; i++)
      frecArities[functions[i].arity()]++;
    cout << "arities (%) ";
    for (int i = 1; i < frecArities.size(); i++)
      cout << "(" << i << ", " << (frecArities[i] * 100) / nfuncs << "), ";
    cout << endl;

    int maxcosts = 0;
    for (int i = 0; i < nfuncs; i++)
      if (costs[i].size() > maxcosts)
        maxcosts = costs[i].size();
    vector<int> frecCosts(maxcosts + 1, 0);
    for (int i = 0; i < nfuncs; i++)
      frecCosts[costs[i].size()]++;
    cout << "costs (%) ";
    for (int i = 1; i < frecCosts.size(); i++)
      cout << "(" << i << ", " << (frecCosts[i] * 100) / nfuncs << "), ";
    cout << endl;
  }
  if (level > 1) {
    cout << "Arities/granularity";
    for (int i = 0; i < functions.size(); i++)
      cout << "(" << functions[i].arity() << ", " << costs[i].size() << "), ";
    cout << endl;

    cout << "Neigbors: ";
    for (int i = 0; i < nvars; i++) {
      cout << i << ":";
      for (int j = 0; j < var2functions[i].size(); j++)
        cout << functions[var2functions[i][j]].arity() << " ";
      cout << endl;
    }
  }
}

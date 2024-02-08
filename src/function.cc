#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "function.hh"
#include "utils.cc"

using namespace std;
using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

Function Function::split(Cost c) {
    Function f_aux = Function(scope, domsize, 0, top);
    for (int i = 0; i < costs.size(); ++i) if (costs[i] == c) {
        f_aux.costs[i] = c;
        costs[i] = 0;
    }
    return f_aux;
}

int Function::tuple2index(const vector<int> &t) const { // flattens a tuple to
                                                        // 1D
  assert(t.size() == scope.size());
  assert(t[0] < domsize[0] and t[0] >= 0);
  int p = t[0];
  for (int i = 1; i < t.size(); i++) {
    assert(t[i] < domsize[i] and t[i] >= 0);
    p = p + t[i] * offset[i];
  }
  return p;
}

vector<int>
Function::index2tuple(int p) const { // unflattens a 1D index to a tuple
  vector<int> t(scope.size());
  for (int i = 0; i < t.size(); i++) {
    t[i] = p % domsize[i];
    p = p / domsize[i];
  }
  return t;
}

/*
bool  Function::indexContains(int p, int var, int val) const {
  vector<int> t=index2tuple(p);
  int pos=var2pos.at(var);
  //assert(pos!=var2pos.end());
  return t[pos] == val;
}
*/

int Function::index2var(int p, int var) const {
  vector<int> t = index2tuple(p);
  int pos = var2pos.at(var);
  // assert(pos!=var2pos.end());
  return t[pos];
}

bool Function::sortedScope() const {
  if (scope.size() == 1)
    return true;
  for (int i = 0; i < scope.size() - 1; i++)
    if (scope[i] > scope[i + 1])
      return false;
  return true;
}

Function Function::sortScope() const {
  vector<int> newscope = scope;
  sort(newscope.begin(), newscope.end());
  vector<int> mapping(scope.size());
  for (int i = 0; i < scope.size(); i++) {
    for (int j = 0; j < scope.size(); j++) {
      if (scope[j] == newscope[i])
        mapping[i] = j;
    }
  }
  vector<int> newdomsize(scope.size());
  for (int i = 0; i < scope.size(); i++)
    newdomsize[i] = domsize[mapping[i]];
  Function newf(newscope, newdomsize, 0, top);
  for (int p = 0; p < costs.size(); p++) {
    Cost c = costs[p];
    vector<int> t = index2tuple(p);
    vector<int> newt(t.size());
    for (int i = 0; i < t.size(); i++)
      newt[i] = t[mapping[i]];
    newf.addCost(newt, c);
  }
  return newf;
}

Function::Function(const vector<int> &s, const vector<int> &d, Cost def,
                   Cost newtop, int sem) {
  // cout << def << ", " << newtop << endl;
  // assert(def <= newtop);
  def = min(def, newtop); // <--- Emma: algunas instancias VAC tienen un def > top
  assert(s.size() == d.size());
  is_hard = (def == 0); // Emma
  top = newtop;
  scope = s;
  for (int i = 0; i < scope.size(); i++) {
    var2pos[scope[i]] = i;
  }
  domsize = d;
  offset = vector<int>(scope.size());
  offset[0] = 1;
  for (int i = 1; i < domsize.size(); i++) {
    offset[i] = offset[i - 1] * domsize[i - 1];
  }
  costs = vector<Cost>(offset[offset.size() - 1] * domsize[s.size() - 1], def);
  // I allow to create functins with some semantics for testing purposes

  assert(sem == 0); // Emma
  /*
  if (sem==1) {
    for(int p=0; p<costs.size(); p++){
      vector<int> t = index2tuple(p);
      Cost c=0;
      for (int i=0;i<t.size();i++) c = c + t[i];
      if (top < c) costs[p] = top;
      else costs[p] = c;
    }
  }
  else if (sem==2) {
    for(int p=0; p<costs.size(); p++){
      vector<int> t = index2tuple(p);
      set<int> s;
      for(int i=0;i<t.size();i++) s.insert(t[i]);
      if(top<s.size()) costs[p]=top;
      else costs[p]= t.size()-s.size();
    }
  }
  */
}

void Function::show(int level) const {
  std::cout << "top " << top << endl;
  std::cout << "scope ";
  for (int i = 0; i < scope.size(); i++)
    std::cout << scope[i] << " ";
  std::cout << std::endl;
  if (level > 0) {
    std::cout << "domsize ";
    for (int i = 0; i < domsize.size(); i++)
      std::cout << domsize[i] << " ";
    std::cout << std::endl;
    std::cout << "costs ";
    for (int i = 0; i < costs.size(); i++)
      std::cout << costs[i] << " ";
    std::cout << std::endl;
  }
  if (level > 1) {
    for (int i = 0; i < costs.size(); i++) {
      vector<int> t = index2tuple(i);
      std::cout << "(";
      for (int j = 0; j < t.size(); j++)
        std::cout << t[j] << ", ";
      std::cout << ": " << costs[i] << ")";
      std::cout << std::endl;
    }
  }
}

bool Function::coversScope(const vector<int> &s) const {
  /*
//inefficiently implemented
for(int i=0;i<scope.size();i++){
  bool found=false;
  for(int j=0;j<s.size();j++) if(scope[i]==s[j]) found=true;
  if(not found) return false;
}
return true;
*/
  for (int var : scope) { // Emma
    int j = 0;
    while (j < s.size() and s[j] != var)
      ++j;
    if (j == s.size())
      return false;
  }
  return true;
}

bool Function::coversScope(const set<int> &s) const { // Emma
  for (int var : scope) {
    if (s.find(var) == s.end())
      return false;
  }
  return true;
}

bool Function::coversAssg(const vector<int> &assg) const {
  for (int i = 0; i < scope.size(); i++) {
    if (assg[scope[i]] == -1)
      return false;
  }
  return true;
}

bool Function::coversAssgButOne(const vector<int> &assg, int &var) const {
  int counter = 0;
  for (int i = 0; i < scope.size(); i++) {
    if (assg[scope[i]] == -1) {
      counter++;
      var = scope[i];
    }
  }
  return counter == 1;
}

void Function::addCost(const vector<int> &t, Cost c) {
  //cout << c << " " << top << endl;
  //assert(c <= top);
  c = min(c, top); // Emma: alguna instancia pasa
  costs[tuple2index(t)] = c;
  updateIsHard(c);
}

bool Function::check() const {
  bool zero = false;
  for (int i = 0; i < costs.size(); i++) {
    if (costs[i] == 0)
      zero = true;
    if (costs[i] > top)
      return false;
  }
  return zero;
}

void Function::updateTop(Cost newTop) {
  assert(newTop <= top);
  assert(newTop > 0);
  if (newTop < top) {
    top = newTop;
    is_hard = true; // Emma
    for (int p = 0; p < costs.size(); p++) {
      if (costs[p] > newTop)
        costs[p] = newTop;
      updateIsHard(costs[p]);
    }
  }
}

Cost Function::getMinCost() const {
  Cost min = top;
  for (int p = 0; p < costs.size(); p++)
    if (costs[p] < min)
      min = costs[p];
  return min;
}

void Function::substractCost(Cost c) {
  is_hard = true; // Emma
  for (int p = 0; p < costs.size(); p++) {
    assert(costs[p] >= c);
    if (costs[p] < top) {
      costs[p] = costs[p] - c;
      updateIsHard(costs[p]); // Emma
    }
  }
}

Cost Function::getCost(const vector<int> &t) const {
  return costs[tuple2index(t)];
}

Cost Function::getCostAssg(const vector<int> &assg) const {
  vector<int> t(scope.size());
  for (int i = 0; i < scope.size(); i++) {
    assert(assg[scope[i]] != -1);
    t[i] = assg[scope[i]];
  }
  return costs[tuple2index(t)];
}

Cost Function::getCostExtended(const vector<int> &t,
                               const vector<int> &t_scope) const {
  assert(t.size() == t_scope.size());

  vector<int> t_this(scope.size(), -1); // will project t over this.scope

  for (int i = 0; i < t.size(); i++) {
    int var = t_scope[i];
    if (var2pos.find(var) != var2pos.end()) { // the variable is in the scope
      t_this[var2pos.at(var)] = t[i];
    }
  }
  for (int i = 0; i < t_this.size(); i++)
    assert(t_this[i] != -1); // it is completely assigned
  return getCost(t_this);
}

int Function::posVar(int var) const {
  if (var2pos.count(var) > 0)
    return var2pos.at(var);
  else
    return -1;
}

vector<Cost> Function::allCosts() const {
  vector<Cost> l;
  for (int p = 0; p < costs.size(); p++) {
    Cost c = costs[p];
    if (c != top and find(l.begin(), l.end(), c) == l.end())
      l.push_back(c);
  }
  sort(l.begin(), l.end());
  return l;
}

vector<int> Function::getScope() const { return scope; }

bool Function::inScope(int var) const { return var2pos.count(var) > 0; }

Function Function::removeVar(int var, Cost fill) const {
  vector<int> newscope;
  vector<int> newdomsize;
  for (int i = 0; i < scope.size(); i++)
    if (scope[i] != var) {
      newscope.push_back(scope[i]);
      newdomsize.push_back(domsize[i]);
    }
  Function f(newscope, newdomsize, fill, top);
  return f;
}

int Function::arity() const { return scope.size(); }

Function Function::condition(int var, int val) const {
  assert(var2pos.find(var) != var2pos.end());
  //cout << "conditioning to var " << var << endl;
  Function f = removeVar(var, 0);
  //cout << "removed" << endl;
  for (int i = 0; i < costs.size(); i++) {
    //cout << "   c " << i << endl;
    vector<int> t = index2tuple(i);
    int pos = var2pos.at(var);
    // assert(pos!=var2pos.end());
    if (t[pos] == val) {

      // if(indexContains(i, var, val)){   // modificado Emma
      // vector<int> t=index2tuple(i);
      // Cost c = getCost(t);
      t.erase(t.begin() + pos); // var2pos.at(var));
      //
      f.addCost(t, costs[i]);
    }
  }
  return f;
}

Function Function::project(int var) const {
  assert(var2pos.find(var) != var2pos.end());
  Function f = removeVar(var, top);
  for (int i = 0; i < costs.size(); i++) {
    vector<int> t = index2tuple(i);
    Cost c = getCost(t);
    t.erase(t.begin() + var2pos.at(var));
    if (f.getCost(t) > c)
      f.addCost(t, c);
  }
  return f;
}

Function Function::join(const Function &f) const {
  assert(top == f.top);
  vector<int> newscope;   // will contain this.scope \cup f.scope
  vector<int> newdomsize; // will contain the corresponding vector of domsizes
  // I do it with the same idea as in mergesort
  int i = 0;
  int j = 0;
  while (i < scope.size() and j < f.scope.size()) {
    if (scope[i] < f.scope[j]) { // the variable is only in this.scope
      newscope.push_back(scope[i]);
      newdomsize.push_back(domsize[i]);
      i++;
    } else {
      if (scope[i] > f.scope[j]) { // the variable is only in f.scope
        newscope.push_back(f.scope[j]);
        newdomsize.push_back(f.domsize[j]);
        j++;
      } else { // the variable is in both scopes
        newscope.push_back(scope[i]);
        newdomsize.push_back(domsize[i]);
        i++;
        j++;
      }
    }
  }
  while (i < scope.size()) {
    newscope.push_back(scope[i]);
    newdomsize.push_back(domsize[i]);
    i++;
  }
  while (j < f.scope.size()) {
    newscope.push_back(f.scope[j]);
    newdomsize.push_back(f.domsize[j]);
    j++;
  }

  Function f2(newscope, newdomsize, 0, top,
              0); // will be the resulting funciton

  for (int p = 0; p < f2.costs.size(); p++) { // for every tuple of f2
    vector<int> t2 = f2.index2tuple(p);
    Cost c_this = getCostExtended(t2, newscope);
    Cost c = f.getCostExtended(t2, newscope);
    Cost added;
    if (c_this + c <= top)
      added = c_this + c;
    else
      added = top;
    f2.addCost(t2, added);
  }

  return f2;
}

int Function::numTuples() const { return costs.size(); }
Cost Function::getCost(int idx) const { return costs[idx]; }
vector<int> Function::getTuple(int idx) const { return index2tuple(idx); }

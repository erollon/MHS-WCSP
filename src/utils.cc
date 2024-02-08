#ifndef UTILS_CC
#define UTILS_CC

#include <vector>
#include <string>
#include <iostream>

using std::vector;
using std::ostream;
using std::string;

typedef long Cost;

template <class T>
ostream &operator<<(ostream &os, const vector<T> &v) {
    os << "[";
    string sep = "";
    for (int i = 0; i < v.size(); ++i) {
        os << sep << v[i];
        sep = ", ";
    }
    os << "]";
    return os;
}

template <class T>
ostream &operator<<(ostream &os, const vector<vector<T>> &v) {
    os << "[";
    string sep = "";
    for (int i = 0; i < v.size(); ++i) {
        os << sep << v[i];
        sep = "\n ";
    }
    os << "]\n";
    return os;
}

template <typename T>
bool operator==(const vector<T> &v1, const vector<T> &v2) {
    assert(v1.size() == v2.size());
    for (int i = 0; i < v1.size(); ++i)
        if (not v1[i] == v2[i]) return false;
    return true;
}

template <typename T>
bool operator<(const vector<T>& a, const vector<T>& b) { //pre: a.size() == b.size()
    for (int i = 0; i < a.size(); i++)
        if (not (a[i] < b[i])) return false;
    return true;
}
template <typename T>
bool operator<=(const vector<T>& a, const vector<T>& b) { //pre: a.size() == b.size()
    for (int i = 0; i < a.size(); i++)
        if (not (a[i] <= b[i])) return false;
    return true;
}
template <typename T>
bool operator>(const vector<T>& a, const vector<T>& b) { //pre: a.size() == b.size()
    for (int i = 0; i < a.size(); i++)
        if (not (a[i] > b[i])) return false;
    return true;
}
template <typename T>
bool operator>=(const vector<T>& a, const vector<T>& b) { //pre: a.size() == b.size()
    for (int i = 0; i < a.size(); i++) {
        if (not (a[i] >= b[i])) return false;
    }
    return true;
}

template <typename T>
T sum(const vector<T>& v) {
    T suma = 0;
    for (int i = 0; i < v.size(); i++) {
        suma += v[i];
    }
    return suma;
}

template <typename T>
vector<T> operator+(const vector<T>& v, T x) {
    vector<T> r(v.size());
    for (int i = 0; i < v.size(); i++) {
        r[i] = v[i] + x;
    }
    return r;
}

template <typename T>
vector<T> operator-(const vector<T>& v, T x) {
    vector<T> r(v.size());
    for (int i = 0; i < v.size(); i++) {
        r[i] = v[i] - x;
    }
    return r;
}

#endif

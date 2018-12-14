#ifndef KM_H
#define KM_H

#include <limits>
#include <vector>

using namespace std;

class KM {

public:
    KM(const vector< vector<int> > & _edges);
    vector<int> solve();

private:

    int nodes_n;
    const vector< vector<int> > edges;
    vector<int> lx, ly, match_x, slack;

};
#endif
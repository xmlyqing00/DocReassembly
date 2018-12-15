#ifndef KM_H
#define KM_H

#include <cmath>
#include <limits>
#include <vector>

#include <utils.h>

using namespace std;

class KM {

public:
    KM(const vector< vector<double> > & _edges);
    vector<int> solve();
    vector<int> cut_loops();
    
    void print_edges();
    void print_matches();

private:

    int nodes_n;
    const vector< vector<double> > edges;
    vector<int> match_x;
    vector<double> lx, ly, slack;
    vector<bool> vx, vy;

    bool Hungarian(int cur_node);

};
#endif
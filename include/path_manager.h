#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include <stripe_pair.h>

using namespace std;

class PathManager {

public:
    int nodes_n {0};
    int sols_n {0};
    map< vector<int>, int> sol_paths;

    PathManager(int _vertices_n, int _sols_n);
    
    void add_sol_path(const vector<int> & sol_path, int sol_cnt=1);
    void print_sol_paths();

    void build_path_graph();
    void print_path_graph();

    vector<StripePair> build_stripe_pairs();

private:
    vector< vector< pair<int, int> > > path_graph;
    vector<StripePair> stripe_pairs;

};

#endif
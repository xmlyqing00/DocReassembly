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
    map< vector<int>, pair<int,int> > sol_paths; // sol; word_cnt, sol_cnt;

    PathManager(int _vertices_n, int _sols_n);
    
    void add_sol_words( const map< vector<int>, int > & sol_words);
    void print_sol_paths();

    void build_path_graph();
    void print_path_graph();

    vector<StripePair> build_stripe_pairs();

private:
    vector< vector< pair<int, double> > > path_graph;
    vector<StripePair> stripe_pairs;

};

#endif
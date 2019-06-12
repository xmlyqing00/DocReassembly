#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <atomic>
#include <algorithm>

#include <stripe_pair.h>

using namespace std;

class PathManager {

public:
    int nodes_n {0};
    int seqs_n {0};
    map< vector<int>, pair<int,int> > seq_paths; // seq; word_cnt, seq_cnt;

    PathManager(int _vertices_n, int _seqs_n);
    ~PathManager();
    
    void add_seq_words( const map< vector<int>, int > & seq_words);
    void print_seq_paths();

    void build_path_graph();
    void print_path_graph();

    vector<StripePair> build_stripe_pairs();

private:
    vector< vector< pair<int, double> > > path_graph;
    vector<StripePair> stripe_pairs;

};

#endif
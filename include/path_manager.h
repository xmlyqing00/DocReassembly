#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <iostream>
#include <vector>
#include <map>

using namespace std;

class PathManager {

public:
    int vertices_n {0};
    map< vector<int>, int> sol_paths;

    PathManager(int _vertices_n);
    
    void add_sol_path(const vector<int> & sol_path);

    void print_sol_paths();

};

#endif
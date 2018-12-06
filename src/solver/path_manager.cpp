#include <path_manager.h>

PathManager::PathManager(int _vertices_n) :
    vertices_n(_vertices_n) {

}

void PathManager::add_sol_path(const vector<int> & sol_path) {
    sol_paths[sol_path]++;
}

void PathManager::print_sol_paths() {

    for (const auto & iter: sol_paths) {
        
        const vector<int> sol_path = iter.first;
        int cnt = iter.second;
        
        cout << "Path: ";
        for (int vertex_idx: sol_path) {
            cout << vertex_idx << " ";
        }

        cout << endl << "Path cnt: " << cnt << endl;
        
    }

}
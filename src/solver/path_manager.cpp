#include <path_manager.h>

PathManager::PathManager(int _nodes_n, int _sols_n) :
    nodes_n(_nodes_n),
    sols_n(_sols_n) {
    
}

PathManager::~PathManager() {
}

void PathManager::add_sol_words(const map< vector<int>, int > & sol_words) {

    for (const auto & iter: sol_words) {

        const vector<int> sol_path = iter.first;
        int word_cnt = iter.second;

        if (sol_paths.find(sol_path) != sol_paths.end()) {
            auto val = sol_paths[sol_path];
            val.first += word_cnt;
            val.second++;
        } else {
            sol_paths[sol_path] = make_pair(word_cnt, 1);
        }

    }                            
    
}

void PathManager::print_sol_paths() {

    for (const auto & iter: sol_paths) {
        
        const vector<int> sol_path = iter.first;
        const auto val = iter.second;
        
        cout << "Path: ";
        for (int vertex_idx: sol_path) {
            cout << vertex_idx << " ";
        }

        cout << endl << "Word cnt: " << val.first << " Sol cnt: " << val.second << endl;
        
    }

}

void PathManager::build_path_graph() {

    path_graph = vector< vector< pair<int, double> > >(nodes_n);

    for (const auto & iter: sol_paths) {
        
        const vector<int> sol_path = iter.first;
        const auto val = iter.second;

        double score = (double)val.first * sol_path.size() / val.second;
        
        for (int i = 1; i < sol_path.size(); i++) {

            bool found_flag = false;
            int cur_node = sol_path[i - 1];
            int next_node = sol_path[i];

            for (auto & edge: path_graph[cur_node]) {
                if (edge.first == next_node) {
                    edge.second += score;
                    found_flag = true;
                    break;
                }
            }
            if (!found_flag) {
                path_graph[cur_node].push_back(make_pair(next_node, score));
            }

        }

    }

}

void PathManager::print_path_graph() {

    for (int i = 0; i < nodes_n; i++) {
        cout << "Node " << i << ": " << endl;
        for (const auto & edge: path_graph[i]) {
            cout << edge.first << " " << edge.second << endl;
        }
    }

}

vector<StripePair> PathManager::build_stripe_pairs() {

    for (int i = 0; i < nodes_n; i++) {
        for (const auto & edge: path_graph[i]) {

            // if (edge.second < 3) continue;
            stripe_pairs.push_back(StripePair(
                i,
                edge.first,
                edge.second,
                1,
                false
            ));
            
        }
    }

    sort(stripe_pairs.begin(), stripe_pairs.end());

    return stripe_pairs;

}
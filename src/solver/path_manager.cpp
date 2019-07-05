#include <path_manager.h>

PathManager::PathManager(int _nodes_n, int _seqs_n) :
    nodes_n(_nodes_n),
    seqs_n(_seqs_n) {
    
}

PathManager::~PathManager() {
}

void PathManager::add_seq_words(const map< vector<int>, int > & seq_words) {

    for (const auto & iter: seq_words) {

        const vector<int> seq_path = iter.first;
        int word_cnt = iter.second;

        if (seq_paths.find(seq_path) != seq_paths.end()) {
            auto val = seq_paths[seq_path];
            val.first += word_cnt;
            val.second++;
        } else {
            seq_paths[seq_path] = make_pair(word_cnt, 1);
        }

    }                            
    
}

void PathManager::print_seq_paths() {

    for (const auto & iter: seq_paths) {
        
        const vector<int> seq_path = iter.first;
        const auto val = iter.second;
        
        cout << "Path: ";
        for (int vertex_idx: seq_path) {
            cout << vertex_idx << " ";
        }

        cout << endl << "Word cnt: " << val.first << " seq cnt: " << val.second << endl;
        
    }

}

void PathManager::build_path_graph() {

    path_graph = vector< vector< pair<int, double> > >(nodes_n);

    for (const auto & iter: seq_paths) {
        
        const vector<int> seq_path = iter.first;
        const auto val = iter.second;

        double score = (double)val.first * seq_path.size() / val.second;
        
        for (int i = 1; i < seq_path.size(); i++) {

            bool found_flag = false;
            int cur_node = seq_path[i - 1];
            int next_node = seq_path[i];

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

    int small_w = 2;

    for (int i = 0; i < nodes_n; i++) {
        for (const auto & edge: path_graph[i]) {

            if (edge.second <= small_w) continue;
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
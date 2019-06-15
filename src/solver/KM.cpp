#include <KM.h>

KM::KM(const vector< vector<double> > & _edges) :
    edges(_edges) {
    
    nodes_n = edges.size();
    lx = vector<double>(nodes_n, 0);
    ly = vector<double>(nodes_n, 0);
    match_x = vector<int>(nodes_n, -1);
    slack = vector<double>(nodes_n, numeric_limits<double>::max());

    for (int i = 0; i < nodes_n; i++) {
        lx[i] = numeric_limits<double>::min();
        for (int j = 0; j < nodes_n; j++) {
            lx[i] = max(lx[i], edges[i][j]);
        }
    }

}

vector<int> KM::solve() {

    for (int i = 0; i < nodes_n; i++) {

        for (int j = 0; j < nodes_n; j++) {
            slack[j] = numeric_limits<int>::max();
        }

        while (true) {
            
            vx = vector<bool>(nodes_n, false);
            vy = vector<bool>(nodes_n, false);
            if (Hungarian(i)) break;

            double d = numeric_limits<double>::max();
            for (int j = 0; j < nodes_n; j++) {
                if (!vy[j]) d = min(d, slack[j]);
            }

            for (int j = 0; j < nodes_n; j++) {
                if (vx[j]) lx[j] -= d;
            }
            for (int j = 0; j < nodes_n; j++) {
                if (vy[j]) ly[j] += d;
            }

        }

    }

    return match_x;

}

vector< vector<int> > KM::cut_loops() {
    
    vector<int> arr;
    vector<bool> visited(nodes_n, false);
    vector< vector<int> > groups;

    for (int i = 0; i < nodes_n; i++) {

        if (visited[i]) continue;

        double match_score = numeric_limits<double>::max();
        int p = i;
        int st_idx = i;

        do {
            if (match_score > edges[match_x[p]][p]) {
                match_score = edges[match_x[p]][p];
                st_idx = p;
            }
            p = match_x[p];
        } while (p != st_idx);

        vector<int> group;
        p = match_x[st_idx];
        do {
            group.push_back(p);
            visited[p] = true;
            p = match_x[p];
        } while (p != match_x[st_idx]);

        reverse(group.begin(), group.end());
        groups.push_back(group);

    }

    return groups;

}

void KM::print_edges() {

    cout << "Edges:" << endl;
    for (int i = 0; i < nodes_n; i++) {
        for (int j = 0; j < nodes_n; j++) {
            cout << edges[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
    
}

void KM::print_matches() {
    
    cout << "Matches:" << endl;
    for (int i = 0; i < nodes_n; i++) {
        cout << match_x[i] << "\t-->\t" << i << endl;
    }
    cout << endl;

}

bool KM::Hungarian(int cur_node) {

    vx[cur_node] = 1;

    for (int i = 0; i < nodes_n; i++) {
        
        if (cur_node == i) continue;
        if (vy[i]) continue;

        double t = lx[cur_node] + ly[i] - edges[cur_node][i];
        if (abs(t) < eps) {
            vy[i] = true;
            if (match_x[i] == -1 || Hungarian(match_x[i])) {
                match_x[i] = cur_node;
                return true;
            }
        } else {
            slack[i] = min(slack[i], t);
        }

    }

    return false;

}
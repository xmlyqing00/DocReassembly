#include <KM.h>

KM::KM(const vector< vector<int> > & _edges) :
    edges(_edges) {
    
    nodes_n = edges.size();
    lx = vector<int>(nodes_n, 0);
    ly = vector<int>(nodes_n, 0);
    match_x = vector<int>(nodes_n, -1);
    slack = vector<int>(nodes_n, numeric_limits<int>::max());

    for (int i = 0; i < nodes_n; i++) {
        lx[i] = numeric_limits<int>::min();
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
            
        }

    }
}
#ifndef STRIPE_PAIR_H
#define STRIPE_PAIR_H

#include <fstream>

using namespace std;

class StripePair {

public:
    int stripe_idx0;
    int stripe_idx1;
    double m_score;
    double ac_prob;
    bool ascending;

    StripePair( int _stripe_idx0, 
                int _stripe_idx1, 
                double _m_score, 
                double _ac_prob=1,
                bool ascending=false);
                
    bool operator < (const StripePair & _sp) const {
        if (ascending) {
            return m_score < _sp.m_score;
        } else {
            return m_score > _sp.m_score;
        }
    }

};

ostream & operator << (ostream & outs, const StripePair & sp);

#endif
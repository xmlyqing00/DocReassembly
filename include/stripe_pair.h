#ifndef STRIPE_PAIR_H
#define STRIPE_PAIR_H

class StripePair {

public:
    int stripe_idx0;
    int stripe_idx1;
    double m_score;

    StripePair(int _stripe_idx0, int _stripe_idx1, double _m_score);
    bool operator < (const StripePair & _sp) const {
        return m_score > _sp.m_score;
    }

};

#endif
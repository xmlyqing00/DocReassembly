#include <stripe_pair.h>

StripePair::StripePair(int _stripe_idx0, int _stripe_idx1, double _m_score, double _ac_prob, bool _ascending) :
    stripe_idx0(_stripe_idx0),
    stripe_idx1(_stripe_idx1),
    m_score(_m_score),
    ac_prob(_ac_prob),
    ascending(_ascending) {
}

ostream & operator << (ostream & outs, const StripePair & sp) {
    outs << sp.stripe_idx0 << " " << sp.stripe_idx1 << " " << sp.m_score << " " << sp.ac_prob;
    return outs;
}
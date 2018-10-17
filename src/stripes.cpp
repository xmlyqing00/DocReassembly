#include <stripes.h>

Stripes::Stripes() {
    stripes_n = 0;
    stripes.clear();
}

void Stripes::push(const cv::Mat & stripe_img) {
    stripes.push_back(stripe_img.clone());
    stripes_n = stripes.size();
}

bool Stripes::reassemble(Composition comp_mode) {
    
    switch (comp_mode) {
        case Stripes::GREEDY:
            return reassemble_greedy();
        default:
            return false;
    }

}

bool Stripes::reassemble_greedy() {
    return true;
}
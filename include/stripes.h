#ifndef STRIPES_H
#define STRIPES_H

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

class Stripes {

public:

    enum Composition {
        GREEDY,
        TSP,
    };

    int stripes_n {0};
    vector<cv::Mat> stripes;

    Stripes();
    void push(const cv::Mat & stripe_img);
    bool reassemble(Composition comp_mode);

private:

    bool reassemble_greedy();
    
};

#endif
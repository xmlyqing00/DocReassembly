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

    double m_metric_word(const cv::Mat & frag0, const cv::Mat & frag1);

private:

    void merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1, cv::Mat & out_frag);

    bool reassemble_greedy();


};

#endif
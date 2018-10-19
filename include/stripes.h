#ifndef STRIPES_H
#define STRIPES_H

#include <numeric>
#include <vector>
#include <deque>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <stripe_pair.h>

using namespace std;

class Stripes {

public:

    enum Composition {
        GREEDY,
        TSP,
    };

    const int word_m_width {100};
    const double conf_thres {80};

    int stripes_n {0};
    vector<cv::Mat> stripes;
    vector<int> comp_idx;
    cv::Mat comp_img;

    Stripes();
    ~Stripes();

    void push(const cv::Mat & stripe_img);
    
    bool reassemble(Composition comp_mode);

    double m_metric_word(const cv::Mat & frag0, const cv::Mat & frag1);

private:

    tesseract::TessBaseAPI * ocr;

    void merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1, cv::Mat & out_frag);

    bool reassemble_greedy();


};

#endif
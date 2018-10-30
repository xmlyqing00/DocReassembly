#ifndef STRIPES_SOLVER_H
#define STRIPES_SOLVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>
#include <vector>
#include <deque>
#include <string>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <stripe_pair.h>
#include <fragment.h>

using namespace std;

class Stripes {

public:

    enum Composition {
        GREEDY,
        TSP,
    };

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};
    const double conf_thres {80};
    string model_path;

    int stripes_n {0};
    vector<cv::Mat> stripes;
    vector<int> comp_idx;
    cv::Mat comp_img;

    Stripes(const string & _model_path);
    ~Stripes();

    void push(const cv::Mat & stripe_img);
    
    bool reassemble(Composition comp_mode);

    double m_metric_word(const Fragment & frag0, const Fragment & frag1);

    void save_result(const string & case_name);

private:

    tesseract::TessBaseAPI * ocr;

    cv::Mat merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1);

    bool reassemble_greedy();

    bool cross_seam(const cv::Rect & bbox, int seam_x);

    bool detect_new_word(   const string & word, 
                            const cv::Rect & bbox, 
                            const Fragment & frag,
                            const int offset_x=0);

    const double overlap_thres {0.3};
    double overlap(const cv::Rect & rect0, const cv::Rect & rect1, const int offset_x=0);

};

#endif
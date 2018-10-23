#ifndef STRIPES_H
#define STRIPES_H

#include <numeric>
#include <vector>
#include <deque>
#include <string>
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

    double m_metric_word(const cv::Mat & frag0, const cv::Mat & frag1);

private:

    tesseract::TessBaseAPI * ocr;

    void merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1, cv::Mat & out_frag);

    bool reassemble_greedy();

    const int extend_p {6};
    cv::Rect extend_bbox(const cv::Rect & o_bbox, const cv::Size & frag_size);

    cv::Rect correct_bbox(  tesseract::TessBaseAPI * correct_bbox, 
                            const cv::Mat & frag,
                            const cv::Rect & o_bbox,
                            const cv::Rect & e_bbox, 
                            const string & word);
    
    bool cross_seam(const cv::Rect & bbox, int seam_x);

};

#endif
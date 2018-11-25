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

#include <ocr_extractor.h>
#include <fragment.h>
#include <utils.h>

using namespace std;

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

class StripesSolver {

public:

    enum Metric {
        PIXEL,
        WORD,
        COMP_EVA
    };

    enum Composition {
        GREEDY,
    };

    const double conf_thres {80};
    string model_path;

    int stripes_n {0};
    vector<cv::Mat> stripes;
    vector<int> comp_idx;
    cv::Mat comp_img;

    OcrExtractor ocr_ectractor;

    StripesSolver(const string & _model_path);
    ~StripesSolver();

    void push(const cv::Mat & stripe_img);
    
    bool reassemble(Metric _metric_mode, Composition comp_mode);

    double m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1);

    double m_metric_word(const cv::Mat & piece0, const cv::Mat & piece1);

    double m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1);

    void save_result(const string & case_name);

private:

    tesseract::TessBaseAPI * ocr;

    Metric metric_mode;
    bool reassemble_greedy();

};

#endif
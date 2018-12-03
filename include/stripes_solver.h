#ifndef STRIPES_SOLVER_H
#define STRIPES_SOLVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>
#include <vector>
#include <deque>
#include <string>
#include <random>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <ocr_extractor.h>
#include <fragment.h>
#include <utils.h>
#include <compatibility_net.h>

using namespace std;

class StripePair {

public:
    int stripe_idx0;
    int stripe_idx1;
    double m_score;
    double ac_prob;

    StripePair(int _stripe_idx0, int _stripe_idx1, double _m_score, double _ac_prob=1);
    bool operator < (const StripePair & _sp) const {
        return m_score > _sp.m_score;
    }

};

ostream & operator << (ostream & outs, const StripePair & sp);

class StripesSolver {

public:

    enum Metric {
        PIXEL,
        WORD,
        COMP_EVA
    };

    enum Composition {
        GREEDY,
        GREEDY_PROBABILITY
    };

    const double conf_thres {80};

    int stripes_n {0};
    vector<cv::Mat> stripes;
    vector<int> composition_order;
    cv::Mat composition_img;

    StripesSolver();
    ~StripesSolver();

    void push(const cv::Mat & stripe_img);

    void m_metric();
    
    bool reassemble(Metric _metric_mode, Composition _composition_mode);

    cv::Mat compose_img(const vector<int> & composition_order);

    void save_result(const string & case_name);

private:

    Metric metric_mode;
    Composition composition_mode;

    vector<StripePair> stripe_pairs;

    // Tesseract
    const string tesseract_model_path {"data/tesseract_model/"};
    tesseract::TessBaseAPI * ocr;

    // Compatibility 
    const double filter_rate = 0.2;
    const int symbols_n = 64;
    const cv::Size cp_net_img_size {64, 64};
    const string saved_model_folder = "data/saved_models/";
    vector<char> symbols;

    OcrExtractor ocr_ectractor;
    CompatibilityNet cp_net;
    Device device {kCPU};

    double m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1);
    double m_metric_word(const cv::Mat & piece0, const cv::Mat & piece1);
    double m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1);
    
    vector<int> reassemble_greedy(bool probability_flag=false);
    vector<int> reassemble_greedy_probability();



};

#endif
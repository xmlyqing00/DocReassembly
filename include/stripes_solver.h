#ifndef STRIPES_SOLVER_H
#define STRIPES_SOLVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <random>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <ocr_extractor.h>
#include <fragment.h>
#include <path_manager.h>
#include <utils.h>
#include <compatibility_net.h>
#include <stripe_pair.h>

using namespace std;

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

    const string puzzle_folder;
    const int stripes_n;
    vector<int> gt_order;
    vector<cv::Mat> stripes;
    vector<int> composition_order;
    cv::Mat composition_img;
    cv::Mat composition_img_seams;

    // Path
    PathManager path_manager;

    StripesSolver(const string & _puzzle_foler, int _stripes_n, int _samples_n);
    ~StripesSolver();

    void m_metric();
    bool reassemble(Metric _metric_mode, Composition _composition_mode);

    cv::Mat compose_img(const vector<int> & composition_order);
    cv::Mat add_seams(const cv::Mat & img, const vector<int> & composition_order);
    void save_result(const string & case_name);

private:

    Metric metric_mode;
    Composition composition_mode;

    vector<StripePair> stripe_pairs;

    // Tesseract
    const string tesseract_model_path {"data/tesseract_model/"};
    tesseract::TessBaseAPI * ocr;
    const double conf_thres {80};

    // Compatibility 
    const double filter_rate = 1;
    const int symbols_n = 64;
    const cv::Size cp_net_img_size {64, 64};
    const string saved_model_folder = "data/saved_models/";
    vector<char> symbols;

    OcrExtractor ocr_ectractor;
    CompatibilityNet cp_net;
    Device device {kCPU};

    // Metric word-path
    int sols_n = 10;
    vector< vector<double> > pixel_graph;

    double m_metric_word(const cv::Mat & piece0, const cv::Mat & piece1);
    double m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1);
    
    vector< vector<int> > reassemble_greedy(bool probability_flag=false);
    void reassemble_greedy_probability();

    cv::Mat word_detection( const cv::Mat & img, 
                            const vector<int> & sol,
                            int sol_cnt=1);
    void finetune_sols(const vector< vector<int> > & composition_orders);

};

#endif
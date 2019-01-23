#ifndef STRIPES_SOLVER_H
#define STRIPES_SOLVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <numeric>
#include <vector>
#include <deque>
#include <string>
#include <omp.h>
#include <map>
#include <random>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <ocr_extractor.h>
#include <fragment.h>
#include <path_manager.h>
#include <utils.h>
#include <KM.h>
#include <compatibility_net.h>
#include <stripe_pair.h>

using namespace std;

class StripesSolver {

public:

    enum Metric {
        PIXEL,
        CHAR,
        WORD,
        COMP_EVA
    };

    enum Composition {
        GREEDY,
        GCOM,
        GREEDY_GCOM,
        GT,
        USER,
    };

    const string puzzle_folder;
    const int stripes_n;
    vector<int> gt_order;
    vector<cv::Mat> stripes;
    vector<int> composition_order;
    cv::Mat composition_img;
    cv::Mat composition_img_seams;
    double composition_score;

    // Path
    PathManager path_manager;

    StripesSolver(const string & _puzzle_foler, int _stripes_n, int _samples_n, bool _real_flag);
    ~StripesSolver();

    void m_metric();
    bool reassemble(Metric _metric_mode, 
                    Composition _composition_mode,
                    const string & case_name, 
                    bool benchmark_flag);

    cv::Mat compose_img(const vector<int> & composition_order, 
                        bool shift_flag=false,
                        vector<int> * sol_x=nullptr);
    cv::Mat add_seams(  const cv::Mat & img, 
                        const vector<int> & composition_order, 
                        bool print_flag=true,
                        const vector<int> * sol_x=nullptr);

private:
    
    Metric metric_mode;
    Composition composition_mode;
    bool real_flag;

    vector<StripePair> stripe_pairs;
    vector<StripePair> stripe_pairs_pixel;

    void save_result(const string & case_name, bool benchmark_flag);

    // Tesseract
    const string tesseract_model_path {"data/tesseract_model/"};
    // tesseract::TessBaseAPI * ocr;
    const double conf_thres {70};

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
    string white_chars, black_chars;

    int sols_n {10};
    int candidate_len {10};
    const int candidate_factor {5};
    vector< vector<double> > pixel_graph;
    vector< vector<double> > pixel_graph2;

    double m_metric_char(const cv::Mat & piece0, const cv::Mat & piece1);
    double m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1);
    void m_metric_word();
    
    vector< vector<int> > reassemble_greedy(bool probability_flag=false);
    void reassemble_GCOM();

    void stochastic_search( vector<int> & sol, const vector< vector<StripePair> > & compose_next);
    cv::Mat word_detection( const cv::Mat & img, 
                            const vector<int> & sol,
                            vector<int> & sol_x);
    void merge_single_sol(vector< vector<int> > & fragments);
    void finetune_sols(const vector< vector<int> > & fragments);

};

static omp_lock_t omp_lock;

#endif
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

// #include <ocr_extractor.h>
#include <fragment.h>
#include <path_manager.h>
#include <utils.h>
#include <KM.h>
// #include <compatibility_net.h>
#include <stripe_pair.h>

using namespace std;

class StripesSolver {

public:

    enum Metric {
        PIXEL,
        CHAR,
        WORD,
        // COMP_EVA
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
    cv::Mat composition_img_bar;
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
                        vector<int> * seq_x=nullptr);
    cv::Mat add_seams(  const cv::Mat & img, 
                        const vector<int> & composition_order, 
                        bool print_flag=true,
                        const vector<int> * seq_x=nullptr);
    cv::Mat add_colorbar(   const cv::Mat & img, 
                            const vector<int> & composition_order, 
                            bool print_flag,
                            const vector<int> * seq_x=nullptr);

private:
    
    vector<time_t> ts_arr;

    Metric metric_mode;
    Composition composition_mode;
    bool real_flag;

    vector<StripePair> stripe_pairs;

    void save_result(const string & case_name, bool benchmark_flag);

    // Tesseract
    const string tesseract_model_path {"data/tesseract_model/"};

    // -- For synthetic cases
    // const double word_conf_thres {70};
    // double lambda0 = 0.3;
    // double lambda1 = 0.5;
    // double filter_rate = 0.7;
    // double U_a = 2;
    // const int candidate_factor {4};
    // -- ------------------

    // -- For physically cases: 
    // -- -- #samples = 10000
    const double word_conf_thres {60};
    double lambda0 = 0.5;
    double lambda1 = 0.7;
    double U_a = 1;
    // const int candidate_factor {5};
    // double filter_rate = 0.2;
    
    // -- ------------------

    // Parameter sets:
    // -- For physically cases: #samples = 10000 Case 2
    // const double word_conf_thres {60};
    // double lambda0 = 0.5;
    // double lambda1 = 0.7;
    // double filter_rate = 0.5;
    // double U_a = 1;
    // const int candidate_factor {3};
    // -- ------------------

    double filter_rate = 0.5;
    const int candidate_factor {4};

    // Metric word-path
    string white_chars, black_chars;

    int candidate_seqs_n {10}; // Placeholder
    int candidate_seq_len {10}; // Placeholder
    vector< vector<double> > low_level_graph;


    double m_metric_char(const cv::Mat & piece0, const cv::Mat & piece1, tesseract::TessBaseAPI * ocr, int idx=0);
    void m_metric_word();

    vector< vector<int> > reassemble_greedy();
    void reassemble_GCOM();

    // For word-path
    void compute_mutual_graph(vector< vector<double> > & mutual_graph);
    void stochastic_search( vector<int> & seq, const vector< vector<StripePair> > & compose_next);
    void compute_word_scores(const vector< vector<int> > & candidate_seqs);
    cv::Mat word_detection( const cv::Mat & img, 
                            const vector<int> & seq,
                            vector<int> & seq_x, 
                            tesseract::TessBaseAPI * ocr);

    // For bigraph
    void compute_bigraph_w(vector< vector<int> > & fragments, vector< vector<double> > & bigraph_w);
    void optimal_match(vector< vector<int> > & fragments);

};

static omp_lock_t omp_lock;

#endif

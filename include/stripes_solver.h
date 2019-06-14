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
    
    Metric metric_mode;
    Composition composition_mode;
    bool real_flag;

    vector<StripePair> stripe_pairs;

    void save_result(const string & case_name, bool benchmark_flag);

    // Tesseract
    const string tesseract_model_path {"data/tesseract_model/"};
    // tesseract::TessBaseAPI * ocr;
    const double word_conf_thres {70};
    double lambda0 = 0.3;
    double lambda1 = 0.5;
    double filter_rate = 0.3;

    // Compatibility 
    // const int symbols_n = 64;
    // const cv::Size cp_net_img_size {64, 64};
    // const string saved_model_folder = "data/saved_models/";
    vector<char> symbols;

    // OcrExtractor ocr_ectractor;
    // CompatibilityNet cp_net;
    // Device device {kCPU};

    // Metric word-path
    string white_chars, black_chars;

    int candidate_seqs_n {10};
    int candidate_seq_len {10};
    const int candidate_factor {5};
    vector< vector<double> > low_level_graph;


    double m_metric_char(const cv::Mat & piece0, const cv::Mat & piece1, tesseract::TessBaseAPI * ocr, int idx=0);
    // double m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1);
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
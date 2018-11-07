#ifndef SQUARES_SOLVER_H
#define SQUARES_SOLVER_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include <utils.h>

using namespace std;

class SquaresSolver {

public:

    enum Metric {
        SYMBOL,
    };

    enum Composition {
        GREEDY,
    };

    enum Splice {
        L,
        R,
        T,
        B
    };

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_SYMBOL};
    const double conf_thres {50};
    const double m_pixel_thres {-20};
    string model_path;

    cv::Size puzzle_size;
    cv::Size square_size;
    int squares_n;
    vector<cv::Mat> squares;

    SquaresSolver(const string & _model_path, const cv::Size & _puzzle_size);
    ~SquaresSolver();

    void push(const cv::Mat & square_img);
    void reassemble();

private:
    tesseract::TessBaseAPI * ocr;

    cv::Mat merge_squares(const cv::Mat & in_img0, const cv::Mat & in_img1, Splice splice);
    bool cross_splice(const cv::Rect & bbox, Splice splice);

    double m_metric_pixel(const cv::Mat & square0, const cv::Mat & square1, Splice splice);
    double m_metric_symbol(const cv::Mat & square0, const cv::Mat & square1, Splice splice);
};

#endif
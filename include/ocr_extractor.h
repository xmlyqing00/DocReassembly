#ifndef OCR_EXTRACTOR_H
#define OCR_EXTRACTOR_H

#include <vector>
#include <algorithm>
#include <queue>
#include <opencv2/opencv.hpp>

using namespace std;

class OcrExtractor {

public:

    enum class Mode {
        DEFAULT,
        CLEAN,
    };

    const Mode mode;
    int roi_idx;
    vector<cv::Mat> roi_arr;

    OcrExtractor(Mode _mode=Mode::DEFAULT);
    void extract_img(const cv::Mat & piece);
    bool has_next();
    cv::Mat next_roi();

};

#endif
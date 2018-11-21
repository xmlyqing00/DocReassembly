#ifndef OCR_EXTRACTOR_H
#define OCR_EXTRACTOR_H

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

class OcrExtractor {

public:

    int ref_imgs_n {0};
    cv::Size ref_img_size;
    int symbol_height {0};
    int blank_height {0};

    OcrExtractor();
    void add_ref_img(const cv::Mat & piece_img);


};

#endif
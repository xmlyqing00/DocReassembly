#ifndef FRAGMENT_H
#define FRAGMENT_H

#include <string>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

using namespace std;

class Fragment {

private:
    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};
    const double conf_thres {80};
    string model_path;

public:
    
    cv::Mat img;
    cv::Size size;
    vector<int> order_idx;
    vector<string> words;
    vector<cv::Rect> bboxs;
    int word_cnt {0};

    Fragment(int idx, const cv::Mat & _img, const string & _model_path); 

};

#endif
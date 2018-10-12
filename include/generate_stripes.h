#ifndef GENERATE_STRIPES_H
#define GENERATE_STRIPES_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

class GenerateStripes {

public:
    cv::Mat ori_img;
    int stripes_n {0};
    cv::Size ori_img_size;

    vector<Mat> stripes;    

    GenerateStripes(string img_path, int stripes_n);

private:

    bool seg_stripes();

};

#endif
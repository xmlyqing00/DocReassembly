#ifndef GENERATE_STRIPES_H
#define GENERATE_STRIPES_H

#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>
#include <opencv2/opencv.hpp>

using namespace std;

class GenerateStripes {

public:
    cv::Mat ori_img;
    int stripes_n {0};
    cv::Size ori_img_size;

    vector<cv::Mat> stripes;    

    GenerateStripes(string img_path, int stripes_n);

    void show_whole_stripes(bool rearrange=false);

private:

    bool seg_stripes();

};

#endif
#ifndef GENERATE_STRIPES_H
#define GENERATE_STRIPES_H

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

#include <opencv2/opencv.hpp>

using namespace std;

class StripesGenerator {

public:
    cv::Mat ori_img;
    int stripes_n {0};
    cv::Size ori_img_size;

    vector<cv::Mat> stripes;    

    StripesGenerator(string img_path, int stripes_n);

    void show_whole_stripes(bool rearrange=false);

    bool save_stripes(const string & output_folder);

private:

    bool seg_stripes();

};

#endif
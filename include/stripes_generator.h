#ifndef STRIPES_GENERATOR_H
#define STRIPES_GENERATOR_H

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

#include <opencv2/opencv.hpp>

using namespace std;

class StripesGenerator {

public:
    cv::Mat ori_img;
    int stripes_n {0};
    cv::Size ori_img_size;
    bool updown_flag = true;

    vector<cv::Mat> stripes;
    vector<int> access_idx;

    StripesGenerator(const string & img_path, int stripes_n, bool _updown_flag=false);

    cv::Mat get_puzzle_img(int gap);

    bool save_puzzle(const string & output_folder);

private:

    bool seg_stripes();

};

#endif
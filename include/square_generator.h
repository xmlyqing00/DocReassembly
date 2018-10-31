#ifndef SQUARE_GENERATOR_H
#define SQUARE_GENERATOR_H

#include <vector>

#include <opencv2/opencv.hpp>

using namespace std;

class SquaresGenerator {

public:
    cv::Mat ori_img;
    int stripes_n {0};
    cv::Size ori_img_size;

    vector<cv::Mat> stripes;
    vector<int> access_idx;

    SquaresGenerator(const string & img_path, int vertical_n);

    cv::Mat get_puzzle_img(int gap);

    bool save_stripes(const string & output_folder);

private:

    bool seg_stripes();
};

#endif
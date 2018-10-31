#ifndef SQUARES_GENERATOR_H
#define SQUARES_GENERATOR_H

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <random>

#include <opencv2/opencv.hpp>

using namespace std;

class SquaresGenerator {

public:
    cv::Mat ori_img;
    cv::Size ori_img_size;
    cv::Size square_size;
    cv::Size puzzle_size;

    int squares_n {0};
    vector<cv::Mat> squares;
    vector<int> access_idx;

    SquaresGenerator(const string & img_path, int vertical_n);

    cv::Mat get_puzzle_img(int gap);

    bool save_puzzle(const string & output_folder);

private:

    bool seg_squares();
};

#endif
#ifndef DEBUG_TOOL_H
#define DEBUG_TOOL_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>

#include <utils.h>

using namespace std;

enum class DebugType {
    Pixel,
};

bool counter_exmple_pixel_metric(   const cv::Mat & piece_root_img,
                                    const cv::Mat & piece_best_img,
                                    const cv::Mat & piece_test_img,
                                    const double & m_score_best,
                                    cv::Mat & canvas);

const string output_folder = "tmp/";

#endif 
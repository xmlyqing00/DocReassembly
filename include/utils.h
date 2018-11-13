#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

enum class PuzzleType {
    STRIPES,
    SQUARES
};

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1);

double m_metric_pixel(const cv::Mat & img0, const cv::Mat & img1);

#endif
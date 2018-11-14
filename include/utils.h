#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>

enum class PuzzleType {
    STRIPES,
    SQUARES
};

const cv::Scalar boundary_color(50, 50, 255);

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1);

double m_metric_pixel(const cv::Mat & img0, const cv::Mat & img1);

cv::Mat merge_imgs(const cv::Mat & in_img0, const cv::Mat & in_img1);

bool cross_seam(const cv::Rect & bbox, int seam_x);

#endif
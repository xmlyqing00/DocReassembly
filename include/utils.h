#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <ctime>
#include <string>
#include <random>
#include <opencv2/opencv.hpp>

using namespace std;

enum class PuzzleType {
    STRIPES,
    SQUARES
};

const double eps = 1e-8;
const cv::Scalar seam_color_red(100, 100, 200);
const cv::Scalar seam_color_green(100, 200, 100);

double avg_vec3b(const cv::Vec3b &v);

int check_pure(double c);

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1);

double m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1, bool shift_flag=false, int idx=0);

cv::Mat merge_imgs( const cv::Mat & in_img0, 
                    const cv::Mat & in_img1, 
                    int & splice_x0,
                    int & splice_x1,
                    bool shift_flag=false);

bool cross_seam(const cv::Rect & bbox, int seam_x);

void print_timestamp();

bool valid_symbol(char ch);

#endif
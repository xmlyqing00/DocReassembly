#include <utils.h>

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1) {

    double diff = 0;
    for (int i = 0; i < 3; i++) {
        diff += abs(v0[i] - v1[i]);
    }
    return diff / 3;

}

double m_metric_pixel(const cv::Mat & img0, const cv::Mat & img1) {

    int x0 = img0.cols - 1;
    int x1 = 0;

    double m_score = 0;
    for (int y = 0; y < img0.rows; y++) {
        m_score += diff_vec3b(  img0.at<cv::Vec3b>(y, x0), 
                                img1.at<cv::Vec3b>(y, x1));
    }

    return -m_score / img0.rows;

}
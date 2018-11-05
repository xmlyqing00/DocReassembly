#include <utils.h>

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1) {

    double diff = 0;
    for (int i = 0; i < 3; i++) {
        diff += abs(v0[i] - v1[i]);
    }
    return diff / 3;

}
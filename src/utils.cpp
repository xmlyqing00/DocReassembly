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

cv::Mat merge_imgs(const cv::Mat & in_img0, const cv::Mat & in_img1) {

    assert(in_img0.rows == in_img1.rows);

    cv::Size out_img_size(in_img0.cols + in_img1.cols, in_img0.rows);
    cv::Mat out_img(out_img_size, CV_8UC3);

    cv::Rect in_img0_roi(0, 0, in_img0.cols, in_img0.rows);
    in_img0.copyTo(out_img(in_img0_roi));

    cv::Rect in_img1_roi(in_img0.cols, 0, in_img1.cols, in_img1.rows);
    in_img1.copyTo(out_img(in_img1_roi));

    return out_img;

}

bool cross_seam(const cv::Rect & bbox, int seam_x) {

    if (bbox.x < seam_x && bbox.x + bbox.width >= seam_x) {
        return true;
    } else {
        return false;
    }

}

void print_timestamp() {
    
    auto now = chrono::system_clock::now();
    time_t cur_time = chrono::system_clock::to_time_t(now);
    
    cout << endl << "Current timestamp: " << ctime(&cur_time) << endl;

}
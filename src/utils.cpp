#include <utils.h>

double avg_vec3b(const cv::Vec3b &v) {
    double avg = 0;
    for (int i = 0; i < 3; i++) {
        avg += v[i];
    }
    return avg / 3;
}

bool check_pure(int c) {
    if (c < 10 || c > 245) return true;
    return false;
}

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1) {

    double avg_vec3b0 = avg_vec3b(v0);
    double avg_vec3b1 = avg_vec3b(v1);
    return abs(avg_vec3b0 - avg_vec3b1);

}

double m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1) {

    assert(piece0.rows == piece1.rows);

    int x0 = piece0.cols - 1;
    int x1 = 0;

    double m_score = 0;
    double avg_pixel_color0 = 0;
    double avg_pixel_color1 = 0;
    for (int y = 0; y < piece0.rows; y++) {
        double avg_vec3b0 = avg_vec3b(piece0.at<cv::Vec3b>(y, x0));
        double avg_vec3b1 = avg_vec3b(piece1.at<cv::Vec3b>(y, x1));
        m_score += abs(avg_vec3b0 - avg_vec3b1);
        avg_pixel_color0 += avg_vec3b0;
        avg_pixel_color1 += avg_vec3b1;
    }

    avg_pixel_color0 /= piece0.rows;
    avg_pixel_color1 /= piece0.rows;
    m_score /= piece0.rows;

    if (check_pure(avg_pixel_color0) && check_pure(avg_pixel_color1)) {
        return -2;
    }
    if (check_pure(avg_pixel_color0) || check_pure(avg_pixel_color1)) {
        return -1;
    }
    return m_score;

}

cv::Mat merge_imgs(const cv::Mat & in_img0, const cv::Mat & in_img1) {

    if (in_img0.empty()) {
        return in_img1;
    }

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

bool valid_symbol(char ch) {
    if (ch >= '0' && ch <= '9') return true;
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    return false;
}
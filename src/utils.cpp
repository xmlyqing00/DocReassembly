#include <utils.h>

double avg_vec3b(const cv::Vec3b &v) {
    double avg = 0;
    for (int i = 0; i < 3; i++) {
        avg += v[i];
    }
    return avg / 3;
}

bool check_pure(int c) {
    if (c < 5 || c > 250) return true;
    return false;
}

double diff_vec3b(const cv::Vec3b & v0, const cv::Vec3b & v1) {

    double avg_vec3b0 = avg_vec3b(v0);
    double avg_vec3b1 = avg_vec3b(v1);
    return abs(avg_vec3b0 - avg_vec3b1);

}

double m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1, bool shift_flag) {

    assert(piece0.rows == piece1.rows);

    int x0 = piece0.cols - 1;
    int x1 = 0;

    if (shift_flag) {
        merge_imgs(piece0, piece1, shift_flag, &x0, &x1);
    }

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

cv::Mat merge_imgs( const cv::Mat & in_img0, 
                    const cv::Mat & in_img1, 
                    bool shift_flag,
                    int * splice_x0, 
                    int * splice_x1) {

    if (shift_flag) {
        *splice_x0 = 0;
        *splice_x1 = 0;
    }

    if (in_img0.empty()) return in_img1;

    assert(in_img0.rows == in_img1.rows);
    
    cv::Size out_img_size(in_img0.cols + in_img1.cols, in_img0.rows);
    cv::Mat out_img = cv::Mat::zeros(out_img_size, CV_8UC3);

    cv::Rect in_img0_roi(0, 0, in_img0.cols, in_img0.rows);
    in_img0.copyTo(out_img(in_img0_roi));

    // cout << in_img0.size() << endl;
    // cout << in_img1.size() << endl;

    if (shift_flag) {

        const double white_thres = 0.8;
        const int color_thres = 200;
        
        int block_h = 20;
        int shift_x0 = 0;
        int shift_x1 = 0;
        int block_cnt = 0;
        int out_width = 0;

        for (int y = 0; y < in_img1.rows; y += block_h) {
            
            if (in_img0.cols > 0) {
                
                int x0;
                for (x0 = in_img0.cols - 1; x0 >= 0; x0--) {
                    const cv::Vec3b & color = in_img0.at<cv::Vec3b>(y, x0);
                    if (color[0] > color_thres && color[1] > color_thres && color[2] > color_thres) break;
                }
                x0++;

                if (y > 0) {
                    shift_x0 = int(round(0.8 * shift_x0 + 0.2 * x0));
                } else {
                    shift_x0 = x0;
                }
                    
            }
            
            if (in_img1.cols > 0) {
                
                int x1;
                for (x1 = 0; x1 < in_img1.cols; x1++) {
                    const cv::Vec3b & color = in_img1.at<cv::Vec3b>(y, x1);
                    if (color[0] > color_thres && color[1] > color_thres && color[2] > color_thres) break;
                }

                if (y > 0) {
                    shift_x1 = int(round(0.8 * shift_x1 + 0.2 * x1));
                } else {
                    shift_x1 = x1;
                }
            }

            if (shift_flag) {
                *splice_x0 += shift_x0 - 1;
                *splice_x1 += shift_x1;
                block_cnt++;
            }

            block_h = min(block_h, in_img1.rows - y);
            cv::Rect roi_src(shift_x1, y, in_img1.cols-shift_x1, block_h);
            cv::Rect roi_dst(shift_x0, y, in_img1.cols-shift_x1, block_h);

            out_width = max(out_width, shift_x0 + in_img1.cols-shift_x1);
            // cout << roi_src << endl;
            // cout << roi_dst << endl;
            in_img1(roi_src).copyTo(out_img(roi_dst));

        }

        if (shift_flag) {
            *splice_x0 /= block_cnt;
            *splice_x1 /= block_cnt;
        }
        out_img = out_img(cv::Rect(0, 0, out_width, out_img.rows));

        // cv::imshow("in0", in_img0);
        // cv::imshow("in1", in_img1);
        // cv::imshow("out", out_img);
        // cv::waitKey();

    } else {

        cv::Rect in_img1_roi(in_img0.cols, 0, in_img1.cols, in_img1.rows);
        in_img1.copyTo(out_img(in_img1_roi));

    }

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
#include <ocr_extractor.h>

OcrExtractor::OcrExtractor() {
    
}

void OcrExtractor::add_ref_img(const cv::Mat & piece_img) {

    if (ref_imgs_n == 0) {
        ref_img_size = piece_img.size();
    }

    ref_imgs_n++;

    cv::Mat gray_img, col_mean;
    cv::cvtColor(piece_img, gray_img, cv::COLOR_BGR2GRAY);
    cv::reduce(gray_img, col_mean, 1, cv::REDUCE_AVG, CV_32FC1);
    cv::normalize(col_mean, col_mean, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::threshold(col_mean, col_mean, 128, 255, cv::THRESH_BINARY);

    int state = 0;
    int index_st = 0;
    int tmp_blank_height = 0;
    int tmp_blank_n = 0;
    int tmp_symbol_height = 0;
    int tmp_symbol_n = 0;

    for (int y = 1; y < ref_img_size.height - 1; y++) {
        
        bool stable_flag = true;
        int self_val = col_mean.at<uchar>(y, 0);
        // cout << "self_val " << self_val << endl;
        
        for (int i = -2; i < 3; i++) {
            if (y + i < 0 || y + i >= ref_img_size.height) continue;
            if (col_mean.at<uchar>(y + i, 0) != self_val) {
                stable_flag = false;
                break;
            }
        }

        if (!stable_flag) continue;

        switch (state) {
            case 0:
                index_st = y;
                if (self_val == 0) {
                    state = -1;
                } else {
                    state = 1;
                }
                break;
            case -1:
                if (self_val == 0) continue;
                tmp_blank_height += y - index_st;
                tmp_blank_n++;
                state = 1;
                index_st = y;
                break;
            case 1:
                if (self_val == 255) continue;
                tmp_symbol_height += y - index_st;
                tmp_symbol_n++;
                state = -1;
                index_st = y;
                break;
        }
        // cout << y << " " << col_mean.at<float>(y, 0) << endl;;
        // vert_hist[y] += col_mean.at<float>(y, 0);
    }

    cout << tmp_blank_height / tmp_blank_n << " " << tmp_symbol_height / tmp_symbol_n << endl;

}
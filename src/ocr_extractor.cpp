#include <ocr_extractor.h>

OcrExtractor::OcrExtractor(Mode _mode) :
    mode(_mode) {
}

void OcrExtractor::extract_img(const cv::Mat & piece) {

    cv::Mat gray_img, col_mean;
    cv::cvtColor(piece, gray_img, cv::COLOR_BGR2GRAY);
    cv::reduce(gray_img, col_mean, 1, cv::REDUCE_AVG, CV_32FC1);
    cv::normalize(col_mean, col_mean, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::threshold(col_mean, col_mean, 200, 255, cv::THRESH_BINARY);

    int state = 0;
    vector<int> blank_y_arr;
    vector<int> symbol_y_arr;

    for (int y = 0; y < piece.rows; y++) {
        
        bool stable_flag = true;
        int self_val = col_mean.at<uchar>(y, 0);
        
        for (int i = -2; i < 3; i++) {
            if (y + i < 0 || y + i >= piece.rows) continue;
            if (col_mean.at<uchar>(y + i, 0) != self_val) {
                stable_flag = false;
                break;
            }
        }

        if (!stable_flag) continue;

        switch (state) {
            case 0: // unknown
                if (self_val == 255) {
                    state = -1;
                    symbol_y_arr.push_back(y);
                } else {
                    state = 1;
                }
                break;
            case -1: // blank
                if (self_val == 255) continue;
                blank_y_arr.push_back(y);
                state = 1;
                break;
            case 1: // symbol
                if (self_val == 0) continue;
                symbol_y_arr.push_back(y);
                state = -1;
                break;
        }

    }

    if (state == -1) blank_y_arr.push_back(piece.rows);

#ifdef DEBUG
    cv::Mat tmp_img = piece.clone();
#endif

    int blank_y_idx = 0;
    vector<int> symbol_block_y_arr;
    for (int symbol_y: symbol_y_arr) {
        while (blank_y_idx < blank_y_arr.size() && blank_y_arr[blank_y_idx] <= symbol_y) blank_y_idx++;
        int symbol_block_y = (symbol_y + blank_y_arr[blank_y_idx]) >> 1;
        symbol_block_y_arr.push_back(symbol_block_y);
        blank_y_idx++;
        
#ifdef DEBUG
        cout <<symbol_y << " " << blank_y_arr[blank_y_idx - 1] << " " << symbol_block_y << endl;
        cv::line(tmp_img, cv::Point(0, symbol_block_y), cv::Point(piece.cols-1, symbol_block_y), cv::Scalar(0, 200, 0), 1);
#endif
    }

#ifdef DEBUG
    cout << endl;
    cv::imshow("piece", piece);
    cv::imshow("tmp", tmp_img);
    cv::waitKey();
#endif

    roi_arr.clear();
    roi_idx = 0;
    int width_half = piece.cols >> 1;

    for (int i = 1; i < (int)symbol_y_arr.size(); i++) {
        
        int y0 = symbol_block_y_arr[i - 1];
        int y1 = symbol_block_y_arr[i];
        int margin = int((y1 - y0) * 0.1);
        y0 = max(0, y0 - margin);
        y1 = min(piece.rows, y1 + margin);
        int height = y1 - y0;

        if (mode == Mode::CLEAN) {
            
            queue< pair<int,int> > que;
            vector< vector<int> > visited();

            for (int y = y0; y < y1; y++) {
                for (int x = width_half - 2; width_half + 2; width_half++) {
                }
            }
        }

        
        int width = min(height, piece.cols);
        int x0 = (piece.cols - width) >> 1;

        cv::Rect roi_rect(x0, y0, width, height);
        roi_arr.push_back(piece(roi_rect).clone());

    }

}

bool OcrExtractor::has_next() {
    return roi_idx < roi_arr.size();
}

cv::Mat OcrExtractor::next_roi() {
    return roi_arr[roi_idx++];
}
#include <squares_solver.h>

SquaresSolver::SquaresSolver(   const string & _model_path, 
                                const cv::Size & _puzzle_size) {

    puzzle_size = _puzzle_size;
    square_size = cv::Size(0, 0);
    model_path = _model_path;
    squares_n = puzzle_size.width * puzzle_size.height;

    ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(model_path.c_str(), "eng", tesseract::OEM_LSTM_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

}

SquaresSolver::~SquaresSolver() {
    ocr->End();
}

void SquaresSolver::push(const cv::Mat & square_img) {
    squares.push_back(square_img.clone());
}

void SquaresSolver::reassemble() {
    
    cout << "[INFO] Reassemble squares puzzle." << endl;
    
    square_size = squares.front().size();

    for (int i = 0; i < squares_n; i++) {
        i = 34;
        for (int k = 0; k < 4; k++) {
            for (int j = 0; j < squares_n; j++) {
                if (i == j) continue;
                double m_pixel_score =  m_metric_pixel(squares[i], squares[j], static_cast<Splice>(k));
                if (m_pixel_score < m_pixel_thres) continue;
                double m_symbol_score = m_metric_symbol(squares[i], squares[j], static_cast<Splice>(k));
                printf("i %d,\t j %d,\t k %d,\t p_score %.1lf,\t s_score %.1lf\n", i, j, k, m_pixel_score, m_symbol_score);
            }
            
        }
    }

}

 cv::Mat SquaresSolver::merge_squares(const cv::Mat & in_img0, const cv::Mat & in_img1, Splice splice) {
    
    cv::Size out_img_size;
    cv::Rect in_img0_roi;
    cv::Rect in_img1_roi;

    switch (splice) {
        case L:
            out_img_size = cv::Size(square_size.width << 1, square_size.height);
            in_img0_roi = cv::Rect(square_size.width, 0, square_size.width, square_size.height);
            in_img1_roi = cv::Rect(0, 0, square_size.width, square_size.height);
            break;
        case R:
            out_img_size = cv::Size(square_size.width << 1, square_size.height);
            in_img0_roi = cv::Rect(0, 0, square_size.width, square_size.height);
            in_img1_roi = cv::Rect(square_size.width, 0, square_size.width, square_size.height);
            break;
        case T:
            out_img_size = cv::Size(square_size.width, square_size.height << 1);
            in_img0_roi = cv::Rect(0, square_size.height, square_size.width, square_size.height);
            in_img1_roi = cv::Rect(0, 0, square_size.width, square_size.height);
            break;
        case B:
            out_img_size = cv::Size(square_size.width, square_size.height << 1);
            in_img0_roi = cv::Rect(0, 0, square_size.width, square_size.height);
            in_img1_roi = cv::Rect(0, square_size.height, square_size.width, square_size.height);
            break;
    }

    cv::Mat out_img(out_img_size, CV_8UC3);
    in_img0.copyTo(out_img(in_img0_roi));
    in_img1.copyTo(out_img(in_img1_roi));

    return out_img;

}

bool SquaresSolver::cross_splice(const cv::Rect & bbox, Splice splice) {

    const double excess_ratio = 0.1;
    int excess_part;

    switch (splice) {
        case L: case R:
            excess_part = (int)(bbox.width * excess_ratio);
            if (bbox.x + excess_part < square_size.width && 
                bbox.x + bbox.width - excess_part > square_size.width) {
                return true;
            } else {
                return false;
            }
        case T: case B:
            excess_part = (int)(bbox.height * excess_ratio);
            if (bbox.y + excess_part < square_size.height && 
                bbox.y + bbox.height - excess_part > square_size.height) {
                return true;
            } else {
                return false;
            }
        default:
            cerr << "[ERRO] Wrong splice" << endl;
            exit(-1);
    }

}

double SquaresSolver::m_metric_pixel(const cv::Mat & square0, const cv::Mat & square1, Splice splice) {

    double m_score = 0;

    switch (splice) {
        case L:
            for (int y = 0; y < square_size.height; y++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(y, 0), square1.at<cv::Vec3b>(y, square_size.width - 1));
            }
            break;
        case R:
            for (int y = 0; y < square_size.height; y++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(y, square_size.width - 1), square1.at<cv::Vec3b>(y, 0));
            }
            break;
        case T:
            for (int x = 0; x < square_size.width; x++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(0, x), square1.at<cv::Vec3b>(square_size.height - 1, x));
            }
            break;
        case B:
            for (int x = 0; x < square_size.width; x++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(square_size.height - 1, x), square1.at<cv::Vec3b>(0, x));
            }
            break;
    }

    return -m_score / square_size.height;

}

double SquaresSolver::m_metric_symbol(const cv::Mat & square0, const cv::Mat & square1, Splice splice) {

    cv::Mat && merged_img = merge_squares(square0, square1, splice);
    double m_score = 0;

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_SYMBOL};

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    // ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, frag0.size.height);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * symbol_iter = ocr->GetIterator();

    if (symbol_iter != 0) {
        do {

            const float conf = symbol_iter->Confidence(tesseract_level);
            if (conf < conf_thres || !symbol_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            symbol_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            if (!cross_splice(o_bbox, splice)) continue;
            const string symbol = symbol_iter->GetUTF8Text(tesseract_level);

            m_score += conf;

#ifdef DEBUG
            cv::rectangle(merged_img, o_bbox, cv::Scalar(0, 0, 200));

            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    symbol.c_str(), conf, symbol_iter->WordIsFromDictionary(), x0, y0, x1, y1);
#endif

        } while (symbol_iter->Next(tesseract_level));
    }

#ifdef DEBUG
    cout << "m_metric_score " << m_score << endl << endl;
    cv::imshow("Merged", merged_img);
    cv::waitKey();
#endif
    
    return m_score;

}
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
    
    square_size = squares.front().size();
    
    for (int i = 0; i < squares_n; i++) {
        for (int j = 0; j < squares_n; j++) {
            if (i == j) continue;
            for (int k = 0; k < 4; k++) {
                double m_pixel_score =  m_metric_pixel(squares[i], squares[j], static_cast<Splice>(k));
                printf("i %d,\t j %d,\t k %d,\t score %.1lf\n", i, j, k, m_pixel_score);
            }
            
        }
    }

}

double SquaresSolver::m_metric_pixel(const cv::Mat & square0, const cv::Mat & square1, SquaresSolver::Splice splice) {

    double m_score = 0;

    switch (splice) {
        case SquaresSolver::L:
            for (int y = 0; y < square_size.height; y++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(y, 0), square1.at<cv::Vec3b>(y, square_size.width - 1));
            }
            break;
        case SquaresSolver::R:
            for (int y = 0; y < square_size.height; y++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(y, square_size.width - 1), square1.at<cv::Vec3b>(y, 0));
            }
            break;
        case SquaresSolver::T:
            for (int x = 0; x < square_size.width; x++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(0, x), square1.at<cv::Vec3b>(square_size.height - 1, x));
            }
            break;
        case SquaresSolver::B:
            for (int x = 0; x < square_size.width; x++) {
                m_score += diff_vec3b(square0.at<cv::Vec3b>(square_size.height - 1, x), square1.at<cv::Vec3b>(0, x));
            }
            break;
    }

    return m_score / square_size.height;

}
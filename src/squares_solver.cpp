#include <squares_solver.h>

SquaresSolver::SquaresSolver(   const string & _model_path, 
                                const cv::Size & _puzzle_size) {

    puzzle_size = _puzzle_size;
    square_size = cv::Size(0, 0);
    model_path = _model_path;

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
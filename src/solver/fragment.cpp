#include <fragment.h>

Fragment::Fragment(int idx, const cv::Mat & _img, const string & _model_path) {
    
    order_idx.push_back(idx);
    model_path = _model_path;
    img = _img;
    size = img.size();

}

void Fragment::ocr_words() {

    tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
    ocr->Init(model_path.c_str(), "eng", tesseract::OEM_LSTM_ONLY);
    ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);
    ocr->Recognize(0);

    tesseract::ResultIterator * word_iter = ocr->GetIterator();

    if (word_iter != 0) {
        do {

            const float conf = word_iter->Confidence(tesseract_level);

            if (conf < conf_thres || !word_iter->WordIsFromDictionary()) {
                continue;
            }

            int x0, y0, x1, y1;
            word_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            bboxs.push_back(cv::Rect(x0, y0, x1 - x0, y1 - y0));
            words.push_back(word_iter->GetUTF8Text(tesseract_level));

        } while (word_iter->Next(tesseract_level));
    }

    ocr->End();

    word_cnt = words.size();

}
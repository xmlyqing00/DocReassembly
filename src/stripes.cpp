#include <stripes.h>

Stripes::Stripes() {
    stripes_n = 0;
    stripes.clear();
    
    ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng")) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

}

Stripes::~Stripes() {
    ocr->End();
}

void Stripes::push(const cv::Mat & stripe_img) {
    stripes.push_back(stripe_img.clone());
    stripes_n = stripes.size();
}

bool Stripes::reassemble(Composition comp_mode) {
    
    switch (comp_mode) {
        case Stripes::GREEDY:
            cout << "Reassemble mode: \t" << "GREEDY" << endl;
            return reassemble_greedy();
        default:
            return false;
    }

}

double Stripes::m_metric_word(const cv::Mat & frag0, const cv::Mat & frag1) {

    cv::Mat merged_frag;
    merge_frags(frag0, frag1, merged_frag);

    int left = frag0.cols - word_m_width / 2;

    ocr->SetImage(merged_frag.data, merged_frag.cols, merged_frag.rows, 3, merged_frag.step);
    ocr->SetRectangle(left, 0, word_m_width, frag0.rows);
    ocr->Recognize(0);
    tesseract::ResultIterator* ri = ocr->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    cv::rectangle(merged_frag, cv::Rect(left, 0, word_m_width, frag0.rows), cv::Scalar(255, 0, 0), 2);

    if (ri != 0) {
        do {
        const char* word = ri->GetUTF8Text(level);
        float conf = ri->Confidence(level);
        int x1, y1, x2, y2;
        ri->BoundingBox(level, &x1, &y1, &x2, &y2);
        printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
                   word, conf, x1, y1, x2, y2);
        delete[] word;
        } while (ri->Next(level));
    }

    cv::imshow("Merged", merged_frag);
    cv::waitKey();

    return 1;

}

void Stripes::merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1, cv::Mat & _out_frag) {

    assert(in_frag0.rows == in_frag1.rows);

    cv::Size out_frag_size(in_frag0.cols + in_frag1.cols, in_frag0.rows);
    cv::Mat out_frag(out_frag_size, CV_8UC3);

    cv::Rect in_frag0_roi(0, 0, in_frag0.cols, in_frag0.rows);
    in_frag0.copyTo(out_frag(in_frag0_roi));

    cv::Rect in_frag1_roi(in_frag0.cols, 0, in_frag1.cols, in_frag1.rows);
    in_frag1.copyTo(out_frag(in_frag1_roi));

    _out_frag = out_frag;

}

bool Stripes::reassemble_greedy() {

    double max_avg_m_score = 0;
    comp_idx = vector<int>(stripes_n, -1);

    for (int i = 0; i < stripes_n; i++) {

        vector<int> inclusion_flag(stripes_n, -1);
        cv::Mat frag = stripes[i].clone();
        inclusion_flag[i] = 0;
        double avg_m_score = 0;
        
        for (int inclusion_cnt = 1; inclusion_cnt < stripes_n; inclusion_cnt++) {
            
            double max_m_score = 0;
            int max_m_score_idx = 0;

            for (int j = 0; j < stripes_n; j++) {

                if (inclusion_flag[j] > -1) continue;
                
                double m_score = m_metric_word(frag, stripes[i]);
                if (max_m_score < m_score) {
                    max_m_score = m_score;
                    max_m_score_idx = j;
                }

            }

            inclusion_flag[max_m_score_idx] = inclusion_cnt;
            avg_m_score += max_m_score;
            merge_frags(frag, stripes[max_m_score_idx], frag);

        }

        avg_m_score /= stripes_n - 1;
        if (max_avg_m_score < avg_m_score) {
            max_avg_m_score = avg_m_score;
            for (int j = 0; j < stripes_n; j++) {
                comp_idx[inclusion_flag[j]] = j;
            }
            comp_img = frag.clone();
        }

    }

    cout << "Best matching score:\t" << max_avg_m_score << endl;

    return true;

}
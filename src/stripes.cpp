#include <stripes.h>

Stripes::Stripes() {
    stripes_n = 0;
    stripes.clear();
    
    ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY)) {
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

    int seam_x = frag0.cols;
    int ocr_left = seam_x - word_m_width / 2;

    cout << "Seam X " << seam_x << endl;

    ocr->SetImage(merged_frag.data, merged_frag.cols, merged_frag.rows, 3, merged_frag.step);
    ocr->SetRectangle(ocr_left, 0, word_m_width, frag0.rows);
    ocr->Recognize(0);
    tesseract::ResultIterator* word_iter = ocr->GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

    double m_metric_score = 0;

    if (word_iter != 0) {
        do {
            const string word = word_iter->GetUTF8Text(level);
            float conf = word_iter->Confidence(level);
            
            // Constraint
            if (conf < conf_thres) continue;
            if (!word_iter->WordIsFromDictionary()) continue;
            int x0, y0, x1, y1;
            word_iter->BoundingBox(level, &x0, &y0, &x1, &y1);
            if (!(x0 < seam_x && x1 > seam_x)) continue;

            m_metric_score += conf * word.length();
            // m_metric_score ++;

            cv::rectangle(merged_frag, cv::Rect(x0, y0, x1-x0+1, y1-y0+1), cv::Scalar(0, 0, 200));

            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    word.c_str(), conf, word_iter->WordIsFromDictionary(), x0, y0, x1, y1);

        } while (word_iter->Next(level));
    }

#ifdef DEBUG
    cout << "m_metric_score " << m_metric_score << endl << endl;
    cv::rectangle(merged_frag, cv::Rect(ocr_left, 0, word_m_width, frag0.rows), cv::Scalar(200, 0, 0));
    cv::line(merged_frag, cv::Point(seam_x, 0), cv::Point(seam_x, frag0.rows), cv::Scalar(200, 0, 0));
    cv::imshow("Merged", merged_frag);
    cv::waitKey();
#endif

    return m_metric_score;

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
                
                double m_score = m_metric_word(frag, stripes[j]);
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

        cout << "avg_m_score: " << avg_m_score << endl;

    }

    cout << "Best matching score:\t" << max_avg_m_score << endl;

    return true;

}
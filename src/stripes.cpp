#include <stripes.h>

Stripes::Stripes() {
    stripes_n = 0;
    
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

    const int seam_x = frag0.cols;
    const int max_m_width = min(frag0.cols, frag1.cols);

    cout << "Seam X " << seam_x << endl;

    ocr->SetImage(merged_frag.data, merged_frag.cols, merged_frag.rows, 3, merged_frag.step);
    ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, frag0.rows);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * word_iter = ocr->GetIterator();
    word_iter->SetBoundingBoxComponents(true, true);

    tesseract::TessBaseAPI * correct_bbox_ocr = new tesseract::TessBaseAPI();
    correct_bbox_ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
    correct_bbox_ocr->SetImage(merged_frag.data, merged_frag.cols, merged_frag.rows, 3, merged_frag.step);

    double m_metric_score = 0;

    if (word_iter != 0) {
        do {
            const string word = word_iter->GetUTF8Text(tesseract_level);
            const float conf = word_iter->Confidence(tesseract_level);
            
            // Conf constraint
            if (conf < conf_thres) continue;

            // Dict constraint
            if (!word_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            word_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            cv::Rect && e_bbox = extend_bbox(o_bbox, merged_frag.size());
            cv::Rect && c_bbox = correct_bbox(correct_bbox_ocr, o_bbox, e_bbox, word);

            if (!(x0 < seam_x && x1 > seam_x)) continue;

            m_metric_score += conf * word.length();
            // m_metric_score ++;

            cv::rectangle(merged_frag, cv::Rect(x0, y0, x1-x0, y1-y0), cv::Scalar(0, 0, 200));

            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    word.c_str(), conf, word_iter->WordIsFromDictionary(), x0, y0, x1, y1);

        } while (word_iter->Next(tesseract_level));
    }

#ifdef DEBUG
    cout << "m_metric_score " << m_metric_score << endl << endl;
    // cv::rectangle(merged_frag, cv::Rect(ocr_left, 0, word_m_width, frag0.rows), cv::Scalar(200, 0, 0));
    // cv::line(merged_frag, cv::Point(seam_x, 0), cv::Point(seam_x, frag0.rows), cv::Scalar(200, 0, 0));
    cv::imshow("Merged", merged_frag);
    cv::waitKey();
#endif

    correct_bbox_ocr->End();

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

cv::Rect Stripes::extend_bbox(const cv::Rect & o_bbox, const cv::Size & frag_size) {

    int x = max(0, o_bbox.x - extend_p);
    int y = max(0, o_bbox.y - extend_p);
    int w = min(frag_size.width, x + o_bbox.width + extend_p * 2);
    int h = min(frag_size.height, y + o_bbox.height + extend_p * 2);

    return cv::Rect(x, y, w, h);

}

cv::Rect Stripes::correct_bbox( tesseract::TessBaseAPI * correct_bbox_ocr, 
                                const cv::Rect & o_bbox,
                                const cv::Rect & e_bbox, 
                                const string & o_word) {

    int x_l = e_bbox.x;
    int x_r = o_bbox.x;

    while (x_l < x_r) {

        int x_m = (x_l + x_r + 1) >> 1;
        int w_m = o_bbox.width - (x_m - x_l) * 2;
        
        correct_bbox_ocr->SetRectangle(x_m, e_bbox.y, w_m, e_bbox.height);
        correct_bbox_ocr->Recognize(0);
        tesseract::ResultIterator * correct_bbox_iter = correct_bbox_ocr->GetIterator();

        const string word = correct_bbox_iter->GetUTF8Text(tesseract_level);
        const float conf = correct_bbox_iter->Confidence(tesseract_level);

        if (word != o_word || conf < conf_thres) {

        }

    }

}

bool Stripes::reassemble_greedy() {

    // Computer matching score for each pair
    vector<StripePair> stripe_pairs;
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n; j++) {
            StripePair sp(i, j, m_metric_word(stripes[i], stripes[j]));
            stripe_pairs.push_back(sp);
        }
    }

    sort(stripe_pairs.begin(), stripe_pairs.end());

    vector<int> stripe_left(stripes_n, -1);
    vector<int> stripe_right(stripes_n, -1);

    double avg_m_score = 0;
    int merged_cnt = 0;
    for (const StripePair & sp: stripe_pairs) {

        if (stripe_right[sp.stripe_idx0] != -1) continue;
        if (stripe_left[sp.stripe_idx1] != -1) continue;

        stripe_right[sp.stripe_idx0] = sp.stripe_idx1;
        stripe_left[sp.stripe_idx1] = sp.stripe_idx0;
        avg_m_score += sp.m_score;
        merged_cnt++;
        
        if (merged_cnt == stripes_n - 1) break;
        
    }

    avg_m_score /= merged_cnt;

    int order_idx = 0;
    while (stripe_left[order_idx] != -1) {
        order_idx = stripe_left[order_idx];
    }
    comp_img = stripes[order_idx];
    comp_idx.push_back(order_idx);

    while (stripe_right[order_idx] != -1) {
        order_idx = stripe_right[order_idx];
        merge_frags(comp_img, stripes[order_idx], comp_img);
        comp_idx.push_back(order_idx);
    }

    cout << "Best matching score:\t" << avg_m_score << endl;

    return true;

}
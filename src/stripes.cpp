#include <stripes.h>

Stripes::Stripes(const string & _model_path) {

    stripes_n = 0;
    model_path = _model_path;
    
    ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(model_path.c_str(), "eng", tesseract::OEM_LSTM_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

    // string white_chars = "";
    // for (int i = 0; i < 10; i++) white_chars += to_string(i);
    // for (int i = 0; i < 26; i++) white_chars += char(int('A') + i);
    // for (int i = 0; i < 26; i++) white_chars += char(int('a') + i);
    // ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());

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

double Stripes::m_metric_word(const Fragment & frag0, const Fragment & frag1) {

    cv::Mat && merged_img = merge_frags(frag0.img, frag1.img);

    const int seam_x = frag0.size.width;
    const int max_m_width = min(frag0.size.width, frag1.size.width);

    cout << "Seam X " << seam_x << endl;

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, frag0.size.height);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * word_iter = ocr->GetIterator();

    tesseract::TessBaseAPI * correct_bbox_ocr = new tesseract::TessBaseAPI();
    correct_bbox_ocr->Init(model_path.c_str(), "eng", tesseract::OEM_LSTM_ONLY);
    // correct_bbox_ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);

    double m_metric_score = 0;

    if (word_iter != 0) {
        do {

            const float conf = word_iter->Confidence(tesseract_level);
            if (conf < conf_thres || !word_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            word_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            // cv::Rect && e_bbox = extend_bbox(o_bbox, merged_img.size());
            if (!cross_seam(o_bbox, seam_x)) continue;

            const string word = word_iter->GetUTF8Text(tesseract_level);
            if (!detect_new_word(word, o_bbox, frag0) || 
                !detect_new_word(word, o_bbox, frag1)) continue;


            // cv::Rect && c_bbox = correct_bbox(correct_bbox_ocr, merged_img, o_bbox, e_bbox, word);
            
            
            m_metric_score += conf * word.length();
            // m_metric_score ++;

            cv::rectangle(merged_img, o_bbox, cv::Scalar(0, 0, 200));
            // cv::rectangle(merged_img, e_bbox, cv::Scalar(0, 200, 0));
            // cv::rectangle(merged_img, c_bbox, cv::Scalar(200, 0, 0));

            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    word.c_str(), conf, word_iter->WordIsFromDictionary(), x0, y0, x1, y1);

            cv::imshow("Merged", merged_img);
            cv::waitKey();

        } while (word_iter->Next(tesseract_level));
    }

#ifdef DEBUG
    cout << "m_metric_score " << m_metric_score << endl << endl;
    // cv::rectangle(merged_img, cv::Rect(ocr_left, 0, word_m_width, frag0.rows), cv::Scalar(200, 0, 0));
    // cv::line(merged_img, cv::Point(seam_x, 0), cv::Point(seam_x, frag0.rows), cv::Scalar(200, 0, 0));
    cv::imshow("Merged", merged_img);
    cv::waitKey();
#endif

    correct_bbox_ocr->End();

    return m_metric_score;

}

cv::Mat Stripes::merge_frags(const cv::Mat & in_frag0, const cv::Mat & in_frag1) {

    assert(in_frag0.rows == in_frag1.rows);

    cv::Size out_frag_size(in_frag0.cols + in_frag1.cols, in_frag0.rows);
    cv::Mat out_frag(out_frag_size, CV_8UC3);

    cv::Rect in_frag0_roi(0, 0, in_frag0.cols, in_frag0.rows);
    in_frag0.copyTo(out_frag(in_frag0_roi));

    cv::Rect in_frag1_roi(in_frag0.cols, 0, in_frag1.cols, in_frag1.rows);
    in_frag1.copyTo(out_frag(in_frag1_roi));

    return out_frag;

}

cv::Rect Stripes::extend_bbox(const cv::Rect & o_bbox, const cv::Size & frag_size) {

    int x = max(0, o_bbox.x - extend_p);
    int y = max(0, o_bbox.y - extend_p / 2);
    int w = min(frag_size.width, o_bbox.width + extend_p * 2);
    int h = min(frag_size.height, o_bbox.height + extend_p);

    return cv::Rect(x, y, w, h);

}

cv::Rect Stripes::correct_bbox( tesseract::TessBaseAPI * correct_bbox_ocr, 
                                const cv::Mat & frag,
                                const cv::Rect & o_bbox,
                                const cv::Rect & e_bbox, 
                                const string & o_word) {
    
    cout << o_bbox << endl;
    cout << e_bbox << endl;

    int x_l = e_bbox.x;
    int x_r = o_bbox.x + o_bbox.width / 4;

    while (x_l < x_r) {

        int x_m = (x_l + x_r + 1) >> 1;

        cout << x_l << " " << x_m << " " << x_r << endl;
        
        const cv::Rect roi_rect(x_m, e_bbox.y, e_bbox.width + e_bbox.x - x_m, e_bbox.height);
        cout << "roi_rect " << roi_rect << "frag size " << frag.size() << endl;
        const cv::Mat roi = frag(roi_rect);
        correct_bbox_ocr->SetImage(roi.data, roi.cols, roi.rows, 3, roi.step);
        correct_bbox_ocr->Recognize(0);
        tesseract::ResultIterator * correct_bbox_iter = correct_bbox_ocr->GetIterator();

        bool match_flag = false;
        do {
            const string word = correct_bbox_iter->GetUTF8Text(tesseract_level);
            cout << word << endl;

            if (word == o_word) {
                match_flag = true;
                break;
            }
        } while (correct_bbox_iter->Next(tesseract_level));

        if (match_flag) {
            x_l = x_m;
        } else{           
            x_r = x_m - 1;
        }

        cv::imshow("roi", roi);
        cv::waitKey();

    }

    return cv::Rect(x_l, e_bbox.y, e_bbox.width + e_bbox.x - x_l, e_bbox.height);

}

bool Stripes::cross_seam(const cv::Rect & bbox, int seam_x) {

    if (bbox.x < seam_x && bbox.x + bbox.width >= seam_x) {
        return true;
    } else {
        return false;
    }

}

bool Stripes::detect_new_word(  const string & word, 
                                const cv::Rect & bbox, 
                                const Fragment & frag) {

    for (int i = 0; i < frag.word_cnt; i++) {
        if (word != frag.words[i]) continue;
        if (overlap(bbox, frag.bboxs[i]) < 0.5) continue;
    }

    return true;

}

double Stripes::overlap(const cv::Rect & rect0, const cv::Rect & rect1) {

    int area0 = rect0.width * rect0.height;
    int area1 = rect1.width * rect1.height;

    int x0 = max(rect0.x, rect1.x);
    int y0 = max(rect0.y, rect1.y);
    int x1 = min(rect0.x + rect0.width, rect1.x + rect1.width);
    
}

bool Stripes::reassemble_greedy() {

    // Init fragment from stripe
    vector<Fragment> frags;
    for (int i = 0; i < stripes_n; i++) {
        frags.push_back(Fragment(i, stripes[i], model_path));
    }

    // Computer matching score for each pair
    vector<StripePair> stripe_pairs;
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            StripePair sp(i, j, m_metric_word(frags[i], frags[j]));
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
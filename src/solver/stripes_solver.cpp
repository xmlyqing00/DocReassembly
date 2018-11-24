#include <stripes_solver.h>

StripesSolver::StripesSolver(const string & _model_path) {

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
    // bool x = ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
    // cout << "set white: " <<  x << endl;
    // string black_chars = ",<.>/?;:\'\"[{]}\\|";
    // ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

}

StripesSolver::~StripesSolver() {
    ocr->End();
}

void StripesSolver::save_result(const string & case_name) {

    const string result_folder = "data/results/"; 
    if (access(result_folder.c_str(), 0) == -1) {
        mkdir(result_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    const string output_path = result_folder + case_name + ".png";
    cv::imwrite(output_path, comp_img);

}

void StripesSolver::push(const cv::Mat & stripe_img) {
    stripes.push_back(stripe_img.clone());
    stripes_n = stripes.size();
}

bool StripesSolver::reassemble(Metric _metric_mode, Composition comp_mode) {
    
    metric_mode = _metric_mode;

    switch (comp_mode) {
        case StripesSolver::GREEDY:
            cout << "Reassemble mode: \t" << "GREEDY" << endl;
            return reassemble_greedy();
        default:
            return false;
    }

}

double StripesSolver::m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1) {

    int x0 = piece0.cols - 1;
    int x1 = 0;

    double m_score = 0;
    for (int y = 0; y < piece0.rows; y++) {
        m_score += diff_vec3b(  piece0.at<cv::Vec3b>(y, x0), 
                                piece1.at<cv::Vec3b>(y, x1));
    }

    return -m_score / piece0.rows;

}

double StripesSolver::m_metric_word(const cv::Mat & piece0, const cv::Mat & piece1) {

    cv::Mat && merged_img = merge_imgs(piece0, piece1);

    const int seam_x = piece0.cols;
    const int max_m_width = min(piece0.cols, piece1.cols);
    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, piece0.rows);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * word_iter = ocr->GetIterator();

    double m_metric_score = 0;

    if (word_iter != 0) {
        do {

            const float conf = word_iter->Confidence(tesseract_level);
            if (conf < conf_thres || !word_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            word_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            if (!cross_seam(o_bbox, seam_x)) continue;

            const string word = word_iter->GetUTF8Text(tesseract_level);
            m_metric_score += conf * word.length();

#ifdef DEBUG
            cv::rectangle(merged_img, o_bbox, cv::Scalar(0, 0, 200));
            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    word.c_str(), conf, word_iter->WordIsFromDictionary(), x0, y0, x1, y1);
#endif

        } while (word_iter->Next(tesseract_level));
    }

#ifdef DEBUG
    cout << "m_metric_score " << m_metric_score << endl << endl;
    cv::imshow("Merged", merged_img);
    cv::waitKey();
#endif

    return m_metric_score;

}

double StripesSolver::m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1) {

    cv::Mat && merged_img = merge_imgs(piece0, piece1);
    const int seam_x = piece0.cols;

    ocr_ectractor.add_img(merged_img);

    while (ocr_ectractor.has_next()) {
        cv::Mat ocr = ocr_ectractor.next_roi();
        cv::imshow("ocr", ocr);
        cv::waitKey();
    }

    return 0;
    
}

bool StripesSolver::reassemble_greedy() {

    // Compute matching score for each pair
    cout << "[INFO] Compute matching score for each pair." << endl;

    vector<StripePair> stripe_pairs;
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n; j++) {
            
            if (i == j) continue;

            double m_score = 0;
            switch (metric_mode) {
                case PIXEL:
                    m_score = m_metric_pixel(stripes[i], stripes[j]);
                    break;
                case WORD:
                    m_score = m_metric_word(stripes[i], stripes[j]);
                    break;
                case COMP_EVA:
                    m_score = m_metric_comp_eva(stripes[i], stripes[j]);
                    break;
            }   
            
#ifdef DEBUG
            cv::Mat && merged_img = merge_imgs(stripes[i].img, stripes[j].img);
            ocr_ectractor.add_img(merged_img);
            printf("Metric i %d, j %d, m %.3lf\n", i, j, m_score);
#endif
            StripePair sp(i, j, m_score);
            stripe_pairs.push_back(sp);

        }
    }

    sort(stripe_pairs.begin(), stripe_pairs.end());

    // Greedy algorithm
    cout << "[INFO] Reassemble the document by greedy algorithm." << endl;

    vector<int> stripe_left(stripes_n, -1);
    vector<int> stripe_right(stripes_n, -1);

    double avg_m_score = 0;
    int merged_cnt = 0;
    for (const StripePair & sp: stripe_pairs) {

        if (stripe_right[sp.stripe_idx0] != -1) continue;
        if (stripe_left[sp.stripe_idx1] != -1) continue;

        int left_most = sp.stripe_idx0;
        while (stripe_left[left_most] != -1) {
            left_most = stripe_left[left_most];
        }

        if (left_most == sp.stripe_idx1) continue;

        stripe_right[sp.stripe_idx0] = sp.stripe_idx1;
        stripe_left[sp.stripe_idx1] = sp.stripe_idx0;
        avg_m_score += sp.m_score;
        merged_cnt++;

        cout << merged_cnt << " " << sp.stripe_idx0 << " " << sp.stripe_idx1 << " " << sp.m_score << endl;
        
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
        comp_img = merge_imgs(comp_img, stripes[order_idx]);
        comp_idx.push_back(order_idx);
    }

    cout << "[INFO] Best matching score:\t" << avg_m_score << endl;

    return true;

}
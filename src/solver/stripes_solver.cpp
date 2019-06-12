#include <stripes_solver.h>

StripesSolver::StripesSolver(const string & _puzzle_folder, int _stripes_n, int _samples_n, bool _real_flag) :
    puzzle_folder(_puzzle_folder),
    stripes_n(_stripes_n),
    candidate_seqs_n(_samples_n),
    path_manager(_stripes_n, _samples_n),
    real_flag(_real_flag) {

    white_chars = "";
    for (int i = 0; i < 10; i++) white_chars += to_string(i);
    for (int i = 0; i < 26; i++) white_chars += char(int('A') + i);
    for (int i = 0; i < 26; i++) white_chars += char(int('a') + i);
    black_chars = ",<.>/?;:\'\"[{]}\\|";

    // Read ground truth order.
    ifstream fin(puzzle_folder + "order.txt", ios::in);
    if (!fin.is_open()) {
        cerr << "[ERRO] " << puzzle_folder + "order.txt" << " does not exist." << endl;
        exit(-1);
    }
    cout << "[INFO] Import groundtruth order." << endl;
    gt_order = vector<int>(stripes_n);
    for (int i = 0; i < stripes_n; i++) fin >> gt_order[i];
    fin.close();

    // Import stripes.
    cout << "[INFO] Import stripes." << endl;
    for (int i = 0; i < stripes_n; i++) {
        const string stripe_img_path = puzzle_folder + to_string(i) + ".png";
        cv::Mat stripe_img = cv::imread(stripe_img_path);
        if (stripe_img.empty()) {
            cerr << "[ERR] Stripe img does not exist." << endl;
            exit(-1); 
        }
        stripes.push_back(move(stripe_img));
    }

    omp_init_lock(&omp_lock);

}

StripesSolver::~StripesSolver() {
    // ocr->End();

    omp_destroy_lock(&omp_lock);
}

bool StripesSolver::reassemble( Metric _metric_mode, 
                                Composition _composition_mode,
                                const string & case_name, 
                                bool benchmark_flag) {
    
    metric_mode = _metric_mode;
    composition_mode = _composition_mode;

    m_metric();

    vector< vector<int> > fragments;
    vector<int> seq_x;

    switch (composition_mode) {
        // 0
        case Composition::GREEDY:
            cout << "[INFO] Composition: Greedy." << endl;

            fragments = reassemble_greedy();
            for (const auto & frag: fragments) {
                composition_order.insert(
                    composition_order.end(), 
                    frag.begin(),
                    frag.end()
                );
            }

            composition_img = compose_img(composition_order, real_flag, &seq_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &seq_x);

            save_result(case_name, benchmark_flag);
            break;
        
        // 1
        case Composition::GCOM:
            cout << "[INFO] Composition: GCOM." << endl;

            reassemble_GCOM();
            
            composition_img = compose_img(composition_order, real_flag, &seq_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &seq_x);
            
            save_result(case_name, benchmark_flag);
            break;
        
        // 2
        case Composition::GREEDY_GCOM:

            // Greedy part
            cout << "[INFO] Composition: Greedy + GCOM: Greedy" << endl;

            composition_mode = Composition::GREEDY;
            fragments = reassemble_greedy();
            for (const auto & frag: fragments) {
                composition_order.insert(
                    composition_order.end(), 
                    frag.begin(),
                    frag.end()
                );
            }

            composition_img = compose_img(composition_order, real_flag, &seq_x);
            if (real_flag) {
                composition_img_bar = add_colorbar(composition_img, composition_order, true, &seq_x);
            } else {
                composition_img_seams = add_seams(composition_img, composition_order, true, &seq_x);
            }

            save_result(case_name, benchmark_flag);

            // GCOM Part
            cout << "[INFO] Composition: Greedy + GCOM: GCOM." << endl;

            composition_mode = Composition::GCOM;
            reassemble_GCOM();

            composition_img = compose_img(composition_order, real_flag, &seq_x);
            if (real_flag) {
                composition_img_bar = add_colorbar(composition_img, composition_order, true, &seq_x);
            } else {
                composition_img_seams = add_seams(composition_img, composition_order, true, &seq_x);
            }
            
            save_result(case_name, benchmark_flag);
            break;

        // 3
        case Composition::GT:

            composition_img = compose_img(gt_order, real_flag, &seq_x);
            composition_img_seams = add_seams(composition_img, gt_order, false, &seq_x);
            save_result(case_name, false);
            break;

        // 4
        case Composition::USER:
        
            // composition_order = vector<int>({19,21,24,26,11,14,2,6,8,1,22,23,15,12,4,17,10,13,16,9,0,7,25,18,3,5});
            composition_order = vector<int>({19, 20, 23, 22, 21, 17, 10, 13, 9, 12, 3, 26, 18, 16, 15, 8, 5, 7, 25, 24, 6, 11, 14, 0, 1, 2, 4});
            
            composition_img = compose_img(composition_order, true, &seq_x);
            // composition_img_seams = add_seams(composition_img, composition_order, false, &seq_x);
            composition_img_bar = add_colorbar(composition_img, composition_order, true, &seq_x);
            save_result(case_name, false);
            break;

        default:
            return false;
    }

    return true;

}

/* *****************
    
    Metric character

****************** */

double StripesSolver::m_metric_char(const cv::Mat & piece0, const cv::Mat & piece1, tesseract::TessBaseAPI * ocr, int idx) {

    int seam_x = piece0.cols;
    int margin_piece1;
    cv::Mat && merged_img = merge_imgs(piece0, piece1, seam_x, margin_piece1, real_flag);

    const int max_m_width = min(piece0.cols, piece1.cols);
    const int bias = real_flag ? 2 : 1;
    const double conf_thres {80};

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, piece0.rows);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * ocr_iter = ocr->GetIterator();

    double m_metric_score = 0;

    if (ocr_iter != 0) {
        do {

            const float conf = ocr_iter->Confidence(tesseract::RIL_SYMBOL);
            if (conf < conf_thres) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            ocr_iter->BoundingBox(tesseract::RIL_SYMBOL, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0 + bias, y0, x1 - x0 - bias * 2, y1 - y0);
            if (!cross_seam(o_bbox, seam_x)) continue;

            const string ocr_char = ocr_iter->GetUTF8Text(tesseract::RIL_SYMBOL);
            m_metric_score += conf;

#ifdef DEBUG
            cv::rectangle(merged_img, o_bbox, cv::Scalar(0, 0, 255));
            printf("word: '%s';  \tconf: %.2f; \t\tBoundingBox: %d,%d,%d,%d; Seam: %d\n",
                    ocr_char.c_str(), conf, x0, y0, x1, y1, seam_x);
#endif

        } while (ocr_iter->Next(tesseract::RIL_SYMBOL));
    }

#ifdef DEBUG
    cv::line(merged_img, cv::Point(seam_x, 0), cv::Point(seam_x, merged_img.rows-1), cv::Scalar(255, 0, 0));
    string merged_path = "tmp/merged/" + to_string(idx) + ".png";
    cv::imwrite(merged_path, merged_img);
#endif

    m_metric_score = 1 - exp(-m_metric_score / 200);

    return m_metric_score;

}

/* *****************

    Metric word-path

****************** */

void StripesSolver::compute_mutual_graph(vector< vector<double> > & mutual_graph) {
    // Check from left to right
    vector< vector<double> > s_l(stripes_n, vector<double>(stripes_n, 0));
    // Check from right to left
    vector< vector<double> > s_r(stripes_n, vector<double>(stripes_n, 0));

    // Compute s_l
    for (int i = 0; i < stripes_n; ++i) {
        
        double score_max = numeric_limits<double>::min();
        double score_min = numeric_limits<double>::max();
        bool valid_flag = false;

        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            if (!real_flag && low_level_graph[i][j] > 1) continue;
            score_max = max(score_max, low_level_graph[i][j]);
            score_min = min(score_min, low_level_graph[i][j]);
            valid_flag = true;
        }

        if (!valid_flag) continue;

        double score_delta = score_max - score_min;
        if (abs(score_delta) < eps) continue;

        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            if (!real_flag && low_level_graph[i][j] > 1) continue;
            s_l[i][j] = (low_level_graph[i][j] - score_min) / score_delta;
        }

    }

    // Compute s_r
    for (int j = 0; j < stripes_n; j++) {
        
        double score_max = numeric_limits<double>::min();
        double score_min = numeric_limits<double>::max();
        bool valid_flag = false;

        for (int i = 0; i < stripes_n; i++) {
            if (i == j) continue;
            if (!real_flag && low_level_graph[i][j] > 1) continue;
            score_max = max(score_max, low_level_graph[i][j]);
            score_min = min(score_min, low_level_graph[i][j]);
            valid_flag = true;
        }

        if (!valid_flag) continue;

        double score_delta = score_max - score_min;
        if (abs(score_delta) < eps) continue;

        for (int i = 0; i < stripes_n; i++) {
            if (i == j) continue;
            if (!real_flag && low_level_graph[i][j] > 1) continue;
            s_r[i][j] = (low_level_graph[i][j] - score_min) / score_delta;
        }

    }

    
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n; j++) {
            mutual_graph[i][j] = s_l[i][j] + s_r[i][j];
        }
    }

}

void StripesSolver::stochastic_search(  vector<int> & seq, const vector< vector<StripePair> > & compose_next) {

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uniform_unit_dist(0, 1);
    uniform_int_distribution<int> uniform_st_dist(0, stripes_n-1);
    vector<bool> stripe_visited(stripes_n, false);
    
    int cur_p = uniform_st_dist(rand_engine);
    seq.push_back(cur_p);
    stripe_visited[cur_p] = true;

    for (int i = 1; i < candidate_seq_len; i++) {
        
        if (compose_next[cur_p].size() == 0) break;

        for (const StripePair & sp: compose_next[cur_p]) {

            if (stripe_visited[sp.stripe_idx1]) continue;
            double rand_p = uniform_unit_dist(rand_engine);
            if (rand_p > sp.ac_prob) continue;

            cur_p = sp.stripe_idx1;
            seq.push_back(cur_p);
            stripe_visited[cur_p] = true;
            break;
        }

        if (seq.size() == i) break;

    }

}

cv::Mat StripesSolver::word_detection(  const cv::Mat & img, 
                                        const vector<int> & seq,
                                        vector<int> & seq_x,
                                        tesseract::TessBaseAPI * ocr) {

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};
    const cv::Scalar color_blue(200, 0, 0);

    ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * ocr_iter = ocr->GetIterator();

    double m_metric_score = 0;
    cv::Mat img_bbox = img.clone();

    if (seq_x.size() == 0) {
        int cum_x = 0;
        for (int i = 0; i < seq.size(); i++) {
            seq_x.push_back(cum_x);
            cum_x += stripes[seq[i]].cols;
        }
    }

    map< vector<int>, int > seq_words;
    int bias = real_flag ? 3 : 1;

    if (ocr_iter != 0) {
        do {
            const float conf = ocr_iter->Confidence(tesseract_level);
            const string word = ocr_iter->GetUTF8Text(tesseract_level);
            if (word.length() < 3 || conf < conf_thres || !ocr_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            ocr_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect bbox(x0 + bias, y0, x1 - x0 - bias * 2, y1 - y0);

            int seq_path_st = upper_bound(seq_x.begin(), seq_x.end(), x0) - seq_x.begin() - 1;
            int seq_path_ed = lower_bound(seq_x.begin(), seq_x.end(), x1) - seq_x.begin();
            
            if (seq_path_ed - seq_path_st > 1) {
                seq_words[vector<int>(seq.begin()+seq_path_st, seq.begin()+seq_path_ed)]++;
            }
#ifdef DEBUG
                cv::rectangle(img_bbox, bbox, color_blue);
                printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                        word.c_str(), conf, ocr_iter->WordIsFromDictionary(), x0, y0, x1, y1);
                cout << endl;
#endif
            


        } while (ocr_iter->Next(tesseract_level));
    }

    omp_set_lock(&omp_lock);
    path_manager.add_seq_words(seq_words);
    omp_unset_lock(&omp_lock);

    return img_bbox;

}

void StripesSolver::compute_word_scores(const vector< vector<int> > & candidate_seqs) {

    int seq_idx = 0;
    int group = 24;

    cout << "Detect words on solution: ";

    #pragma omp parallel for
    for (int i = 0; i < group; i++) {

        tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
        if (ocr->Init(tesseract_model_path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY)) {
            cerr << "Could not initialize tesseract." << endl;
            exit(-1);
        }

        ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
        ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

        ocr->SetVariable("language_model_penalty_non_freq_dict_word", "5");
        ocr->SetVariable("language_model_penalty_non_dict_word", "1");

        for (int j = i; j < candidate_seqs.size(); j += group) {

            const auto & seq = candidate_seqs[j];
            vector<int> seq_x;
            cv::Mat composition_img = compose_img(seq, real_flag, &seq_x);
            // cv::imwrite("tmp/comp_" + to_string(seq_idx + 1) + ".png", composition_img);
            cv::Mat tmp_img = word_detection(composition_img, seq, seq_x, ocr);

#ifdef DEBUG
            cout << seq_idx << endl;
            tmp_img = add_seams(tmp_img, seq, false, &seq_x);
            ++seq_idx;
            cv::imwrite("tmp/seq_" + to_string(seq_idx) + ".png", tmp_img);
#else
            ++seq_idx;
            if (seq_idx % 100 == 0) {
                cout << seq_idx << " " << flush;
            }
#endif
        }

        ocr->End();

    }
}

void StripesSolver::m_metric_word() {

    // Compute mutual low level graph
    vector< vector<double> > mutual_graph = vector< vector<double> >(stripes_n, vector<double>(stripes_n, 0));
    compute_mutual_graph(mutual_graph);

    // Compute stripe_pairs
    double U_a = 1;
    if (real_flag) U_a = 1.5;

    int filters_n = min(int(stripes_n * filter_rate), stripes_n - 1);
    cout << "Preserve stripes:    \t" << filters_n << endl;

    vector< vector<StripePair> > compose_next;
    int occur_cnt = 0;

    for (int i = 0; i < stripes_n; i++) {
        
        vector<StripePair> next_pairs;
        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            next_pairs.push_back(StripePair(i, j, mutual_graph[i][j]));
        }

        sort(next_pairs.begin(), next_pairs.end());
        
        double worst_score = next_pairs[filters_n].m_score;
        for (int j = filters_n; j >= 0; j--) {
            if (next_pairs[j].m_score > eps) {
                worst_score = next_pairs[j].m_score;
                break; 
            }
        }

        if (abs(worst_score) < eps) {
            compose_next.push_back(vector<StripePair>());
            continue;
        }

        if (!real_flag) {
            next_pairs.erase(next_pairs.begin() + filters_n, next_pairs.end());
        }

        int gt_next_idx = -1;
        for (int j = 0; j < stripes_n - 1; j++) {
            if (gt_order[j] == i) {
                gt_next_idx = gt_order[j + 1];
                break;
            }
        }
        
        for (auto & stripe_pair: next_pairs) {
            double alpha = U_a * (stripe_pair.m_score / worst_score - 1);
            double exp_alpha = exp(alpha);
            stripe_pair.ac_prob = (exp_alpha - 1) / (exp_alpha + 1);


            if (gt_next_idx == stripe_pair.stripe_idx1) {
                occur_cnt++;
                cout << stripe_pair << endl;
            }

        }

        compose_next.push_back(move(next_pairs));
            
    }

    printf("Stripe pairs in searching space: %d / %d\n", occur_cnt, stripes_n-1);

    map< vector<int>, bool > seq_visited;
    vector< vector<int> > candidate_seqs;

    candidate_seq_len = stripes_n / candidate_factor;
    cout << "Candidate length:   \t" << candidate_seq_len << endl;
    cout << "[INFO] Search candidate seqs." << endl;
    
    vector< vector<int> > composition_cnt(stripes_n, vector<int>(stripes_n, 0));

    while (candidate_seqs.size() < candidate_seqs_n) {

        vector<int> seq;
        stochastic_search(seq, compose_next);
        
        if (seq.size() > candidate_seq_len / 2 && seq_visited[seq] == false) {
            seq_visited[seq] = true;
            candidate_seqs.push_back(seq);
            for (int i = 1; i < seq.size(); i++) {
                composition_cnt[seq[i-1]][seq[i]]++;
            }
        }
    }

    occur_cnt = 0;
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n - 1; j++) {
            if (gt_order[j] == i) {
                cout << "Occurence cnt " << i << " " << gt_order[j+1] << " " << composition_cnt[i][gt_order[j+1]] << endl;
                if (composition_cnt[i][gt_order[j+1]] > 0) occur_cnt++;
                break;
            }
        }
    }

    printf("Candidate sequence in word detection: %d / %d\n", occur_cnt, stripes_n-1);
    compute_word_scores(candidate_seqs);
    

    cout << endl;

    path_manager.build_path_graph();

#ifdef DEBUG
    path_manager.print_seq_paths();
    path_manager.print_path_graph();
#endif

    stripe_pairs_pixel = stripe_pairs;
    stripe_pairs = path_manager.build_stripe_pairs();


}

/* *****************

    Metric manager

****************** */

void StripesSolver::m_metric() {

    // Compute matching score for each pair
    cout << "[INFO] Compute matching score for each pair." << endl;

    low_level_graph = vector< vector<double> >(stripes_n, vector<double>(stripes_n, 0));

    #pragma omp parallel for
    for (int i = 0; i < stripes_n; i++) {
        
        tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
        if (ocr->Init(tesseract_model_path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY)) {
            cerr << "Could not initialize tesseract." << endl;
            exit(-1);
        }

        ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
        ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

        ocr->SetVariable("language_model_penalty_non_freq_dict_word", "5");
        ocr->SetVariable("language_model_penalty_non_dict_word", "1");

        for (int j = 0; j < stripes_n; j++) {
            
            if (i == j) continue;

            double m_score_p, m_score_c, m_score = 0;

            switch (metric_mode) {
                case Metric::PIXEL:
                    m_score = m_metric_pixel(stripes[i], stripes[j], real_flag);
                    break;
                case Metric::CHAR:
                    m_score = m_metric_char(stripes[i], stripes[j], ocr);
                case Metric::WORD:
                    m_score_p = m_metric_pixel(stripes[i], stripes[j], real_flag, i * stripes_n + j);
                    m_score_c = m_metric_char(stripes[i], stripes[j], ocr, i * stripes_n + j);
                    if (m_score_p > 1) {
                        m_score = m_score_p;
                    } else {
                        m_score = lambda_ * m_score_c + (1 - lambda_) * m_score_p;
                    }
#ifdef DEBUG
                    printf("Idx: %d, metric (%d, %d)\tc: %.3lf, p: %.3lf, low: %.3lf\n", i*stripes_n + j, i, j, m_score_c, m_score_p, m_score);
#endif
                    break;
                default:
                    break;
            }   

            low_level_graph[i][j] = m_score;

            omp_set_lock(&omp_lock);
            if (m_score < 1) {
                stripe_pairs.push_back(StripePair(i, j, m_score, 1.0, false));
            }
            omp_unset_lock(&omp_lock);

        }

        ocr->End();

    }

    sort(stripe_pairs.begin(), stripe_pairs.end());
#ifdef DEBUG
    // for (const StripePair & sp : stripe_pairs) {
    //     cout << sp << endl;
    // }
#endif

    if (metric_mode == Metric::WORD) {
        cout << "[INFO] Calculate word metric." << endl;
        m_metric_word();
    };

}

/* *****************

   Reassemble greedy

****************** */

vector< vector<int> > StripesSolver::reassemble_greedy() {

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uniform_unit_dist(0, 1);

    vector<int> stripe_left(stripes_n, -1);
    vector<int> stripe_right(stripes_n, -1);

    int merged_cnt = 0;

    for (const StripePair & sp: stripe_pairs) {

        if (composition_mode == Composition::GREEDY && sp.m_score > 2) continue;

        if (stripe_right[sp.stripe_idx0] != -1) continue;
        if (stripe_left[sp.stripe_idx1] != -1) continue;

        int left_most = sp.stripe_idx0;
        while (stripe_left[left_most] != -1) {
            left_most = stripe_left[left_most];
        }

        if (left_most == sp.stripe_idx1) continue;

        stripe_right[sp.stripe_idx0] = sp.stripe_idx1;
        stripe_left[sp.stripe_idx1] = sp.stripe_idx0;
        merged_cnt++;

        if (merged_cnt == stripes_n - 1) break;
        
    }

    vector< vector<int> > fragments;
    vector<bool> visited(stripes_n, false);

    for (int order_idx = 0; order_idx < stripes_n; order_idx++) {

        if (visited[order_idx]) continue;

        int cur_idx = order_idx;
        while (stripe_left[cur_idx] != -1) {
            cur_idx = stripe_left[cur_idx];
        }

        vector<int> seq;
        seq.push_back(cur_idx);
        visited[cur_idx] = true;

        int splice_x0, splice_x1;

        while (stripe_right[cur_idx] != -1) {
            cur_idx = stripe_right[cur_idx];
            seq.push_back(cur_idx);
            visited[cur_idx] = true;
        }

        fragments.push_back(move(seq));

    }

    return fragments;

}

/* *****************

    Reassemble GCOM

****************** */

void StripesSolver::pair_merge_frags(vector< vector<int> > & fragments) {

    cout << "[INFO] Pair merge fragments." << endl;
#ifdef DEBUG
    cout << "Fragments before:" << endl;
    for (int i = 0; i < fragments.size(); i++) {
        cout << "[" << i << "]\t";
        for (const int & x: fragments[i]) cout << x << " ";
        cout << endl;
    }
    cout << endl;
#endif

    vector<cv::Mat> frag_imgs;
    for (const auto & fragment: fragments) {
        cv::Mat frag_img = compose_img(fragment, real_flag);
        frag_imgs.push_back(frag_img);

#ifdef DEBUG
        cv::imshow("frag", frag_img);
        cv::waitKey();
#endif
    }





    for (int i = 0; i < fragments.size(); i++) {

        for (int j = 0; j < fragments.size(); j++) {

            if (i == j) continue;



        }

    }

}

void StripesSolver::finetune_sols(const vector< vector<int> > & fragments) {

    cout << "[INFO] Finetune composition orders." << endl;

    cout << "Fragments:" << endl;
    for (int i = 0; i < fragments.size(); i++) {
        cout << "[" << i << "]\t";
        for (const int & x: fragments[i]) cout << x << " ";
        cout << endl;
    }
    cout << endl;

    if (fragments.size() == 1) {
        composition_order = fragments[0];
        return;
    }

    vector<int> in_nodes, out_nodes;
    for (const auto & sol: fragments) {
        in_nodes.push_back(sol.front());
        out_nodes.push_back(sol.back());
    }

    int nodes_n = in_nodes.size();
    vector< vector<double> > edges(nodes_n, vector<double>(nodes_n));

    for (int i = 0; i < out_nodes.size(); i++) {
        for (int j = 0; j < in_nodes.size(); j++) {
            if (i == j) continue;
            if (low_level_graph[out_nodes[i]][in_nodes[j]] < -eps) {
                edges[i][j] = -100;
            } else {
                edges[i][j] = -low_level_graph[out_nodes[i]][in_nodes[j]];
            }
            
        }
    }

    KM KM_solver(edges);
    KM_solver.solve();
    vector<int> arr = KM_solver.cut_loops();

#ifdef DEBUG
    KM_solver.print_edges();
    KM_solver.print_matches();
#endif 

    vector<int> final_sol;
    for (int frag_idx: arr) {
        final_sol.insert(
            final_sol.end(), 
            fragments[frag_idx].begin(),
            fragments[frag_idx].end()
        );
    }

#ifdef DEBUG
    cout << "Final sol metric scores:" << endl;
    for (int i = 1; i < stripes_n; i++) {
        cout << final_sol[i-1] << "\t->\t" << final_sol[i] << "\t" << low_level_graph[final_sol[i-1]][final_sol[i]] << endl;
    }
    cout << endl;
#endif

    composition_order = final_sol;

}

void StripesSolver::reassemble_GCOM() {

    vector< vector<int> > && fragments = reassemble_greedy();

    pair_merge_frags(fragments);
    finetune_sols(fragments);

}

/* *****************

    Utils

****************** */

cv::Mat StripesSolver::compose_img( const vector<int> & composition_order, 
                                    bool shift_flag, 
                                    vector<int> * seq_x) {
    
    cv::Mat composition_img;
    int x0, x1;
    for (int i = 0; i < composition_order.size(); i++) {
        // if (i == 8) {
            // cv::imshow("comp", composition_img);
            // cv::waitKey();
        // }
        composition_img = merge_imgs(composition_img, stripes[composition_order[i]], x0, x1, shift_flag);
        if (shift_flag) seq_x->push_back(x0);
    }

    return composition_img;

}

cv::Mat StripesSolver::add_seams(   const cv::Mat & img, 
                                    const vector<int> & composition_order, 
                                    bool print_flag,
                                    const vector<int> * seq_x) {

    cv::Mat img_seams = img.clone();
    int col = stripes[composition_order[0]].cols;
    cv::Scalar seam_color;

    int correct_cnt = 0;

    for (int i = 1; i < composition_order.size(); i++) {

        for (int j = 0; j < stripes_n; j++) {
            if (gt_order[j] != composition_order[i-1]) continue;
            if (j == stripes_n - 1 || gt_order[j+1] != composition_order[i]) {
                seam_color = seam_color_red;
            } else {
                seam_color = seam_color_green;
                correct_cnt++;
            }
            break;
        }

        if (seq_x->size() == 0) {
            cv::line(img_seams, cv::Point(col, 0), cv::Point(col, img.rows), seam_color);
            col += stripes[composition_order[i]].cols;
        } else {
            cv::line(img_seams, cv::Point((*seq_x)[i], 0), cv::Point((*seq_x)[i], img.rows), seam_color);
        }
        

    }

    if (print_flag) {
        composition_score = (double)correct_cnt / (int(composition_order.size()) - 1);
        printf("Composition Score: %.6lf\n", composition_score);
    }

    return img_seams;

}

cv::Mat StripesSolver::add_colorbar(const cv::Mat & img, 
                                    const vector<int> & composition_order, 
                                    bool print_flag,
                                    const vector<int> * seq_x) {

    const int bar_h = min(30, stripes[composition_order[0]].cols - 2);
    const int indicator_w = bar_h;

    cv::Size img_size = img.size();
    img_size.height += bar_h;
    cv::Mat img_colorbar(img_size, CV_8UC3, cv::Scalar(255, 255, 255));
    img.copyTo(img_colorbar(cv::Rect(0, bar_h, img.cols, img.rows)));
    
    int col = stripes[composition_order[0]].cols;
    cv::Scalar bar_color;

    int correct_cnt = 0;

    for (int i = 1; i < composition_order.size(); i++) {

        for (int j = 0; j < stripes_n; j++) {
            if (gt_order[j] != composition_order[i-1]) continue;
            if (j == stripes_n - 1 || gt_order[j+1] != composition_order[i]) {
                bar_color = seam_color_red;
            } else {
                bar_color = seam_color_green;
                correct_cnt++;
            }
            break;
        }

        if (seq_x->size() == 0) {
            img_colorbar(cv::Rect(col-indicator_w/2, 0, indicator_w, bar_h)) = bar_color;
            col += stripes[composition_order[i]].cols;
        } else {
            img_colorbar(cv::Rect((*seq_x)[i]-indicator_w/2, 0, indicator_w, bar_h)) = bar_color;
        }
        
    }

    if (print_flag) {
        composition_score = (double)correct_cnt / (int(composition_order.size()) - 1);
        printf("Composition Score: %.6lf\n", composition_score);
    }

    return img_colorbar;

}

void StripesSolver::save_result(const string & case_name, bool benchmark_flag) {

    const string result_folder = "data/results/"; 
    if (access(result_folder.c_str(), 0) == -1) {
        mkdir(result_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    string output_path = 
        result_folder + case_name + "_" + to_string(stripes_n) + "_" +
        to_string(static_cast<int>(metric_mode)) + "_" + 
        to_string(static_cast<int>(composition_mode)) +
        (composition_mode == Composition::GCOM ? "_s" + to_string(candidate_seqs_n) : "");
        
    cv::imwrite(output_path + ".png", composition_img);
    if (!composition_img_seams.empty()) {
        cv::imwrite(output_path + "_seams.png", composition_img_seams);
    }
    if (!composition_img_bar.empty()) {
        cv::imwrite(output_path + "_bar.png", composition_img_bar);
    }
    

    if (benchmark_flag) {
        ofstream fout("data/scores_ori/" + case_name + ".txt", ios::app);
        fout << composition_score << " ";
        if ((stripes_n == 40 || stripes_n == 60) && composition_mode == Composition::GCOM) {
            fout << endl;
        }
        fout.close();
    }
    
}
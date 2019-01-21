#include <stripes_solver.h>

StripesSolver::StripesSolver(const string & _puzzle_folder, int _stripes_n, int _samples_n, bool _real_flag) :
    puzzle_folder(_puzzle_folder),
    stripes_n(_stripes_n),
    sols_n(_samples_n),
    path_manager(_stripes_n, sols_n),
    real_flag(_real_flag) {

    // ocr = new tesseract::TessBaseAPI();
    // if (ocr->Init(tesseract_model_path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY)) {
    //     cerr << "Could not initialize tesseract." << endl;
    //     exit(-1);
    // }

//     string white_chars = "";
//     for (int i = 0; i < 10; i++) white_chars += to_string(i);
//     for (int i = 0; i < 26; i++) white_chars += char(int('A') + i);
//     for (int i = 0; i < 26; i++) white_chars += char(int('a') + i);
//     bool x = ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
//     string black_chars = ",<.>/?;:\'\"[{]}\\|";
//     ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

//     ocr->SetVariable("language_model_penalty_non_freq_dict_word", "5");
//     ocr->SetVariable("language_model_penalty_non_dict_word", "1");

// #ifdef DEBUG
//     FILE * file = fopen("tmp/variables.txt", "w");
//     ocr->PrintVariables(file);
//     fclose(file);
// #endif

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

void StripesSolver::save_result(const string & case_name, bool benchmark_flag) {

    const string result_folder = "data/results/"; 
    if (access(result_folder.c_str(), 0) == -1) {
        mkdir(result_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    string output_path = 
        result_folder + case_name + "_" + to_string(stripes_n) + "_" +
        to_string(static_cast<int>(metric_mode)) + "_" + 
        to_string(static_cast<int>(composition_mode)) +
        (composition_mode == Composition::GCOM ? "_s" + to_string(sols_n) : "");
        
    cv::imwrite(output_path + ".png", composition_img);
    cv::imwrite(output_path + "_seams.png", composition_img_seams);

    if (benchmark_flag) {
        ofstream fout("data/scores_ori/" + case_name + ".txt", ios::app);
        fout << composition_score << " ";
        if ((sols_n == 3200 || sols_n == 4000) && composition_mode == Composition::GCOM) {
            fout << endl;
        }
        fout.close();
    }
    
}

bool StripesSolver::reassemble( Metric _metric_mode, 
                                Composition _composition_mode,
                                const string & case_name, 
                                bool benchmark_flag) {
    
    metric_mode = _metric_mode;
    composition_mode = _composition_mode;

    m_metric();

    vector< vector<int> > fragments;
    vector<int> sol_x;

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

            composition_img = compose_img(composition_order, real_flag, &sol_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &sol_x);

            save_result(case_name, benchmark_flag);
            break;
        
        // 1
        case Composition::GCOM:
            cout << "[INFO] Composition: GCOM." << endl;

            reassemble_GCOM();
            
            composition_img = compose_img(composition_order, real_flag, &sol_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &sol_x);

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

            composition_img = compose_img(composition_order, real_flag, &sol_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &sol_x);

            save_result(case_name, benchmark_flag);

            // GCOM Part
            cout << "[INFO] Composition: GCOM." << endl;

            composition_mode = Composition::GCOM;
            reassemble_GCOM();

            composition_img = compose_img(composition_order, real_flag, &sol_x);
            composition_img_seams = add_seams(composition_img, composition_order, true, &sol_x);
            
            save_result(case_name, benchmark_flag);
            break;

        // 3
        case Composition::GT:

            composition_img = compose_img(gt_order, real_flag, &sol_x);
            composition_img_seams = add_seams(composition_img, gt_order, false, &sol_x);
            save_result(case_name, false);
            break;

        // 4
        case Composition::USER:
            // composition_order = vector<int>({19,21,24,26,11,14,2,6,8,1,22,23,15,12,4,17,10,13,16,9,0,7,25,18,3,5});
            composition_order = vector<int>({8,0,5,7,9,10,26,6,17,11,14,15,16,12,18,13,3,19,20,1,23,22,21,24,4,2,25});
            
            composition_img = compose_img(composition_order, true, &sol_x);
            composition_img_seams = add_seams(composition_img, composition_order, false, &sol_x);
            save_result(case_name, false);
            break;

        default:
            return false;
    }

    return true;

}

cv::Mat StripesSolver::compose_img( const vector<int> & composition_order, 
                                    bool shift_flag, 
                                    vector<int> * sol_x) {
    
    cv::Mat composition_img;
    int x0, x1;
    for (int i = 0; i < composition_order.size(); i++) {
        composition_img = merge_imgs(composition_img, stripes[composition_order[i]], shift_flag, &x0, &x1);
        if (shift_flag) sol_x->push_back(x0);
    }

    return composition_img;

}

cv::Mat StripesSolver::add_seams(   const cv::Mat & img, 
                                    const vector<int> & composition_order, 
                                    bool print_flag,
                                    const vector<int> * sol_x) {

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

        if (sol_x->size() == 0) {
            cv::line(img_seams, cv::Point(col, 0), cv::Point(col, img.rows), seam_color);
            col += stripes[composition_order[i]].cols;
        } else {
            cv::line(img_seams, cv::Point((*sol_x)[i], 0), cv::Point((*sol_x)[i], img.rows), seam_color);
        }
        

    }

    if (print_flag) {
        composition_score = (double)correct_cnt / (int(composition_order.size()) - 1);
        printf("Composition Score: %.6lf\n", composition_score);
    }

    return img_seams;

}

double StripesSolver::m_metric_char(const cv::Mat & piece0, const cv::Mat & piece1) {

    int seam_x = piece0.cols;
    int margin_piece1;
    cv::Mat && merged_img = merge_imgs(piece0, piece1, real_flag, &seam_x, &margin_piece1);

    tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(tesseract_model_path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

    string white_chars = "";
    for (int i = 0; i < 10; i++) white_chars += to_string(i);
    for (int i = 0; i < 26; i++) white_chars += char(int('A') + i);
    for (int i = 0; i < 26; i++) white_chars += char(int('a') + i);
    bool x = ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
    string black_chars = ",<.>/?;:\'\"[{]}\\|";
    ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

    ocr->SetVariable("language_model_penalty_non_freq_dict_word", "5");
    ocr->SetVariable("language_model_penalty_non_dict_word", "1");

    const int max_m_width = min(piece0.cols, piece1.cols);
    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_SYMBOL};

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    ocr->SetRectangle(seam_x - max_m_width, 0, max_m_width << 1, piece0.rows);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * ocr_iter = ocr->GetIterator();

    double m_metric_score = 0;

    if (ocr_iter != 0) {
        do {

            const float conf = ocr_iter->Confidence(tesseract_level);
            if (conf < conf_thres) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            ocr_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            if (!cross_seam(o_bbox, seam_x)) continue;

            const string ocr_char = ocr_iter->GetUTF8Text(tesseract_level);
            m_metric_score += conf * ocr_char.length();

#ifdef DEBUG
            // cv::rectangle(merged_img, o_bbox, cv::Scalar(0, 0, 200));
            // printf("word: '%s';  \tconf: %.2f; \t\tBoundingBox: %d,%d,%d,%d;\n",
                    // ocr_char.c_str(), conf, x0, y0, x1, y1);
#endif

        } while (ocr_iter->Next(tesseract_level));
    }

#ifdef DEBUG
    // cout << "m_metric_score " << m_metric_score << endl << endl;
    // cv::imshow("Merged", merged_img);
    // cv::waitKey();
#endif

    ocr->End();

    return -m_metric_score;

}

double StripesSolver::m_metric_comp_eva(const cv::Mat & piece0, const cv::Mat & piece1) {

    double m_pixel_score = m_metric_pixel(piece0, piece1);

    cv::Mat && merged_img = merge_imgs(piece0, piece1);
    const int seam_x = piece0.cols;

    cv::imshow("merged", merged_img);

    ocr_ectractor.extract_img(merged_img);

    int idx = 0;
    while (ocr_ectractor.has_next()) {

        cv::Mat ocr = ocr_ectractor.next_roi();
        cv::resize(ocr, ocr, cp_net_img_size);

        Tensor img_tensor = torch::from_blob(ocr.data, {ocr.rows, ocr.cols, 3}, kByte);
        img_tensor = img_tensor.permute({2, 0, 1}).toType(kFloat32).div_(255).unsqueeze(0);
        img_tensor = img_tensor.to(device);
        

        Tensor output = cp_net.forward(img_tensor);
        int class_idx = output.argmax(1).item<int>();

        cout << output << endl;
        cout << "pred " << class_idx << " " << symbols[class_idx] << endl;

        cv::imshow("ocr", ocr);
        cv::imwrite("tmp/ocr_" + to_string(idx++) + ".png", ocr);
        cv::waitKey();

    }

    return 0;

}

void StripesSolver::m_metric_word() {

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
            if (!real_flag && pixel_graph[i][j] < 0) continue;
            score_max = max(score_max, pixel_graph[i][j]);
            score_min = min(score_min, pixel_graph[i][j]);
            valid_flag = true;
        }

        if (!valid_flag) continue;

        double score_delta = score_max - score_min;
        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            if (!real_flag && pixel_graph[i][j] < 0) continue;
            if (abs(score_delta) < eps) continue;
            double score = (pixel_graph[i][j] - score_min) / score_delta;
            s_l[i][j] = exp(-score);
        }

    }

    // Compute s_r
    for (int j = 0; j < stripes_n; j++) {
        
        double score_max = numeric_limits<double>::min();
        double score_min = numeric_limits<double>::max();
        bool valid_flag = false;

        for (int i = 0; i < stripes_n; i++) {
            if (i == j) continue;
            if (!real_flag && pixel_graph[i][j] < 0) continue;
            score_max = max(score_max, pixel_graph[i][j]);
            score_min = min(score_min, pixel_graph[i][j]);
            valid_flag = true;
        }

        if (!valid_flag) continue;

        double score_delta = score_max - score_min;
        for (int i = 0; i < stripes_n; i++) {
            if (i == j) continue;
            if (!real_flag && pixel_graph[i][j] < 0) continue;
            if (abs(score_delta) < eps) continue;
            double score = (pixel_graph[i][j] - score_min) / score_delta;
            s_r[i][j] = exp(-score);
        }

    }

    // Compute pixel graph2
    pixel_graph2 = vector< vector<double> >(stripes_n, vector<double>(stripes_n, 0));
    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n; j++) {
            pixel_graph2[i][j] = s_l[i][j] + s_r[i][j];
        }
    }

    // Compute stripe_pairs
    double U_a = 3;
    if (real_flag) U_a = 1.1;

    vector< vector<StripePair> > compose_next;
    for (int i = 0; i < stripes_n; i++) {
        
        vector<StripePair> next_pairs;
        for (int j = 0; j < stripes_n; j++) {
            if (i == j) continue;
            next_pairs.push_back(StripePair(i, j, pixel_graph2[i][j]));
        }

        sort(next_pairs.begin(), next_pairs.end());
        
        double worst_score = next_pairs[stripes_n/2].m_score;
        //double worst_score;
        //for (int j = next_pairs.size() - 1; j >= 0; j--) {
        //    if (next_pairs[j].m_score > eps) {
        //        worst_score = next_pairs[j].m_score;
        //        break; 
        //    }
        //}
        if (abs(worst_score) < eps) {
            compose_next.push_back(vector<StripePair>());
            continue;
        }

        next_pairs.erase(next_pairs.begin() + stripes_n / 2, next_pairs.end());
        for (int j = 0; j < next_pairs.size(); j++) {
            double alpha = U_a * (next_pairs[j].m_score / worst_score - 1);
            double exp_alpha = exp(alpha);
            next_pairs[j].ac_prob = (exp_alpha - 1) / (exp_alpha + 1);
        }

        compose_next.push_back(move(next_pairs));
            
    }

    map< vector<int>, bool > sol_visited;
    vector< vector<int> > candidate_sols;

    candidate_len = stripes_n / candidate_factor;
    cout << "Candidate length:   \t" << candidate_len << endl;
    cout << "[INFO] Search candidate sols." << endl;
    
    vector< vector<int> > composition_cnt(stripes_n, vector<int>(stripes_n, 0));

    while (candidate_sols.size() < sols_n) {

        vector<int> sol;
        stochastic_search(sol, compose_next);
        
        if (sol.size() > candidate_len / 2 && sol_visited[sol] == false) {
            sol_visited[sol] = true;
            candidate_sols.push_back(sol);
            for (int i = 1; i < sol.size(); i++) {
                composition_cnt[sol[i-1]][sol[i]]++;
            }
        }
    }

    for (int i = 0; i < stripes_n; i++) {
        for (int j = 0; j < stripes_n - 1; j++) {
            if (gt_order[j] == i) {
                cout << "Occurence cnt " << i << " " << gt_order[j+1] << " " << composition_cnt[i][gt_order[j+1]] << endl;
                break;
            }
        }
    }

    int sol_idx = 0;

    cout << "Detect words on solution: ";

    
    #pragma omp parallel for num_threads(20)
    // for (const auto & sol: candidate_sols) {
    for (int i = 0; i < candidate_sols.size(); i++) {

        const auto & sol = candidate_sols[i];
        
        ++sol_idx;
        
        vector<int> sol_x;
        cv::Mat composition_img = compose_img(sol, real_flag, &sol_x);
        cv::Mat tmp_img = word_detection(composition_img, sol, sol_x);

#ifdef DEBUG
        cout << sol_idx << endl;
        tmp_img = add_seams(tmp_img, sol, false, &sol_x);
        cv::imwrite("tmp/sol_" + to_string(sol_idx) + ".png", tmp_img);
        // cv::imshow("Tmp img", tmp_img);
        // cv::waitKey();
#else
        if (sol_idx % 100 == 0) {
            cout << sol_idx << " " << flush;
        }

#endif
    }
    cout << endl;

    path_manager.build_path_graph();

#ifdef DEBUG
    path_manager.print_sol_paths();
    path_manager.print_path_graph();
#endif

    stripe_pairs_pixel = stripe_pairs;
    stripe_pairs = path_manager.build_stripe_pairs();


}

void StripesSolver::m_metric() {

    // Compute matching score for each pair
    cout << "[INFO] Compute matching score for each pair." << endl;

    int filters_n = min(int(stripes_n * filter_rate), stripes_n - 1);
    // int filters_n = stripes_n - 1;

    cout << "Preserve stripes:    \t" << filters_n << endl;

    if (metric_mode == COMP_EVA) {

        const string model_path = saved_model_folder + "best.pt";
        if (access(model_path.c_str(), 0) == -1) {
            cerr << "Model file: " << model_path << " does not exist!" << endl;
            exit(-1);
        }
        serialize::InputArchive input_archive;
        input_archive.load_from(model_path);

        cp_net.load(input_archive);

        for (int i = 0; i < 10; i++) symbols.push_back('0' + i);
        for (int i = 0; i < 26; i++) symbols.push_back('A' + i);
        for (int i = 0; i < 26; i++) symbols.push_back('a' + i);
        symbols.push_back('.');
        symbols.push_back('?');

        DeviceType device_type;
        if (cuda::is_available()) {
            cout << "CUDA available! Training on GPU" << endl;
            device_type = kCUDA;
        } else {
            cout << "Training on CPU" << endl;
            device_type = kCPU;
        }
        device = Device(device_type);
        cp_net.to(device);

        for (int i = 0; i < stripes_n; i++) {

            vector<StripePair> candidates;
            for (int j = 0; j < stripes_n; j++) {
                if (i == j) continue;
                double m_score = m_metric_pixel(stripes[i], stripes[j]);
                candidates.push_back(StripePair(i, j, m_score));
            }

            sort(candidates.begin(), candidates.end());

            for (int j = 0; j < filters_n; j++) {
                cout << j << " " << candidates[j].m_score << endl;
                double m_score = m_metric_comp_eva(stripes[i], stripes[candidates[j].stripe_idx1]);
                StripePair sp(i, candidates[j].stripe_idx1, m_score);
                stripe_pairs.push_back(sp);
            }

        }

    
    } else {

        pixel_graph = vector< vector<double> >(stripes_n, vector<double>(stripes_n, 0));
        double m_score_p, m_score_c;

        #pragma omp parallel for num_threads(20)
        for (int i = 0; i < stripes_n; i++) {
            
            #pragma omp parallel for num_threads(20)
            for (int j = 0; j < stripes_n; j++) {
                
                if (i == j) continue;

                double m_score = 0;
                switch (metric_mode) {
                    case Metric::PIXEL:
                        m_score = m_metric_pixel(stripes[i], stripes[j], real_flag);
                        break;
                    case Metric::CHAR:
                        m_score = m_metric_char(stripes[i], stripes[j]);
                    case Metric::WORD:
                        m_score_p = m_metric_pixel(stripes[i], stripes[j], real_flag);
                        //if (m_score_p < -eps) {
                        //    m_score_p = 100;
                        //}
                        if (real_flag) {
                            m_score_c = m_metric_char(stripes[i], stripes[j]);
                            m_score = 2 * m_score_p + m_score_c;
                        } else {
                            m_score = m_score_p;
                        }
#ifdef DEBUG
                        cout << "Init " << i << " " << j << " " << m_score << endl;
#endif
                        break;
                    default:
                        break;
                }   

                omp_set_lock(&omp_lock);
                pixel_graph[i][j] = m_score;
                if (m_score_p > -eps) {
                    stripe_pairs.push_back(StripePair(i, j, m_score, 1.0, true));
                }
                omp_unset_lock(&omp_lock);

            }

        }
    }

    sort(stripe_pairs.begin(), stripe_pairs.end());
#ifdef DEBUG
    for (const StripePair & sp : stripe_pairs) {
        cout << sp << endl;
    }
#endif

    if (metric_mode == Metric::WORD) {
        cout << "[INFO] Calculate word metric." << endl;
        m_metric_word();
    };

}

vector< vector<int> > StripesSolver::reassemble_greedy(bool probability_flag) {

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uniform_unit_dist(0, 1);

    vector<int> stripe_left(stripes_n, -1);
    vector<int> stripe_right(stripes_n, -1);

    int merged_cnt = 0;
    vector<StripePair> stripe_pairs_rest;

    double thres_score = 0;
    if (composition_mode == Composition::GCOM) {
        if (metric_mode == Metric::PIXEL || metric_mode == Metric::CHAR) {
            thres_score = 0.8 * stripe_pairs.back().m_score;
        }
#ifdef DEBUG
        cout << "Threshold score: " << thres_score << endl;
#endif
    }

    for (const StripePair & sp: stripe_pairs) {

        if (sp.m_score < -eps) continue;

        if (abs(thres_score) > eps) {
            if (composition_mode == Composition::GCOM) {
                if (metric_mode == Metric::PIXEL || metric_mode == Metric::CHAR) {
                    if (sp.m_score > thres_score) break;
                }
            }
        }

        if (stripe_right[sp.stripe_idx0] != -1) continue;
        if (stripe_left[sp.stripe_idx1] != -1) continue;

        if (probability_flag) {
            double rand_f = uniform_unit_dist(rand_engine);
            if (rand_f > sp.ac_prob) {
                stripe_pairs_rest.push_back(sp);
                continue;
            }
        }

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

    if (probability_flag && merged_cnt < stripes_n - 1) {
        for (const StripePair & sp: stripe_pairs_rest) {

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
    }

    vector< vector<int> > fragments;
    vector<bool> visited(stripes_n, false);

    for (int order_idx = 0; order_idx < stripes_n; order_idx++) {

        if (visited[order_idx]) continue;

        int cur_idx = order_idx;
        while (stripe_left[cur_idx] != -1) {
            cur_idx = stripe_left[cur_idx];
        }

        vector<int> sol;
        sol.push_back(cur_idx);
        visited[cur_idx] = true;

        while (stripe_right[cur_idx] != -1) {
            cur_idx = stripe_right[cur_idx];
            composition_img = merge_imgs(composition_img, stripes[cur_idx]);
            sol.push_back(cur_idx);
            visited[cur_idx] = true;
        }

        fragments.push_back(move(sol));

    }

    return fragments;

}

void StripesSolver::reassemble_GCOM() {

    vector< vector<int> > && fragments = reassemble_greedy(false);

    merge_single_sol(fragments);
    finetune_sols(fragments);

}

void StripesSolver::stochastic_search(  vector<int> & sol, const vector< vector<StripePair> > & compose_next) {

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uniform_unit_dist(0, 1);
    uniform_int_distribution<int> uniform_st_dist(0, stripes_n-1);
    vector<bool> stripe_visited(stripes_n, false);
    
    int cur_p = uniform_st_dist(rand_engine);
    sol.push_back(cur_p);
    stripe_visited[cur_p] = true;

    for (int i = 1; i < candidate_len; i++) {
        
        if (compose_next[cur_p].size() == 0) break;

        for (const StripePair & sp: compose_next[cur_p]) {

            if (stripe_visited[sp.stripe_idx1]) continue;
            double rand_p = uniform_unit_dist(rand_engine);
            if (rand_p > sp.ac_prob) continue;

            cur_p = sp.stripe_idx1;
            sol.push_back(cur_p);
            stripe_visited[cur_p] = true;
            break;
        }

        if (sol.size() == i) break;

    }

}

cv::Mat StripesSolver::word_detection(  const cv::Mat & img, 
                                        const vector<int> & sol,
                                        vector<int> & sol_x) {

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};
    const cv::Scalar color_blue(200, 0, 0);

    tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(tesseract_model_path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

    string white_chars = "";
    for (int i = 0; i < 10; i++) white_chars += to_string(i);
    for (int i = 0; i < 26; i++) white_chars += char(int('A') + i);
    for (int i = 0; i < 26; i++) white_chars += char(int('a') + i);
    bool x = ocr->SetVariable("tessedit_char_whitelist", white_chars.c_str());
    string black_chars = ",<.>/?;:\'\"[{]}\\|";
    ocr->SetVariable("tessedit_char_blacklist", black_chars.c_str());

    ocr->SetVariable("language_model_penalty_non_freq_dict_word", "5");
    ocr->SetVariable("language_model_penalty_non_dict_word", "1");

    ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * ocr_iter = ocr->GetIterator();

    double m_metric_score = 0;
    cv::Mat img_bbox = img.clone();

    if (sol_x.size() == 0) {
        int cum_x = 0;
        for (int i = 0; i < sol.size(); i++) {
            sol_x.push_back(cum_x);
            cum_x += stripes[sol[i]].cols;
        }
    }
    

    map< vector<int>, int > sol_words;

    if (ocr_iter != 0) {
        do {
            const float conf = ocr_iter->Confidence(tesseract_level);
            const string word = ocr_iter->GetUTF8Text(tesseract_level);
            if (word.length() < 3 || conf < conf_thres || !ocr_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            ocr_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect bbox(x0 + 3, y0, x1 - x0 - 6, y1 - y0);

            int sol_path_st = upper_bound(sol_x.begin(), sol_x.end(), x0) - sol_x.begin() - 1;
            int sol_path_ed = lower_bound(sol_x.begin(), sol_x.end(), x1) - sol_x.begin();
            
            if (sol_path_ed - sol_path_st > 1) {
                sol_words[vector<int>(sol.begin()+sol_path_st, sol.begin()+sol_path_ed)]++;
#ifdef DEBUG
                printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                        word.c_str(), conf, ocr_iter->WordIsFromDictionary(), x0, y0, x1, y1);
                cout << endl;
#endif
            }

#ifdef DEBUG
            cv::rectangle(img_bbox, bbox, color_blue);
#endif

        } while (ocr_iter->Next(tesseract_level));
    }

    omp_set_lock(&omp_lock);
    path_manager.add_sol_words(sol_words);
    omp_unset_lock(&omp_lock);

    ocr->End();

    return img_bbox;

}

void StripesSolver::merge_single_sol(vector< vector<int> > & fragments) {

    cout << "[INFO] Merge single composition order." << endl;
#ifdef DEBUG
    cout << "Fragments:" << endl;
    for (int i = 0; i < fragments.size(); i++) {
        cout << "[" << i << "]\t";
        for (const int & x: fragments[i]) cout << x << " ";
        cout << endl;
    }
    cout << endl;
#endif

    for (int i = 0; i < fragments.size(); i++) {

        if (fragments[i].size() > 1) continue;
        
        double min_score = numeric_limits<double>::max();
        int end_id = 0;
        int single_node = fragments[i][0];

        for (int j = 0; j < fragments.size(); j++) {
            if (i == j) continue;
            if (pixel_graph[single_node][fragments[j].front()] < -1) continue;
            if (min_score > pixel_graph[single_node][fragments[j].front()]) {
                min_score = pixel_graph[single_node][fragments[j].front()];                
                end_id = -j;
            }

            if (pixel_graph[fragments[j].back()][single_node] < -1) continue;
            if (min_score > pixel_graph[fragments[j].back()][single_node]) {
                min_score = pixel_graph[fragments[j].back()][single_node];                
                end_id = j;
            }
        }

        cout << single_node << " " << end_id << endl;
        int composition_idx = abs(end_id);

        if (end_id == 0) {
            cout << "Single node: " << single_node << " preserved." << endl;
            continue;
        }

        if (end_id > 0) {
            fragments[composition_idx].push_back(single_node);
        } else {
            auto p_st = fragments[composition_idx].begin();
            fragments[composition_idx].insert(p_st, single_node);
        }

        fragments.erase(fragments.begin() + i);
        i--;

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
            if (pixel_graph[out_nodes[i]][in_nodes[j]] < -eps) {
                edges[i][j] = -100;
            } else {
                edges[i][j] = -pixel_graph[out_nodes[i]][in_nodes[j]];
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
        cout << final_sol[i-1] << "\t->\t" << final_sol[i] << "\t" << pixel_graph[final_sol[i-1]][final_sol[i]] << endl;
    }
    cout << endl;
#endif

    composition_order = final_sol;

}
#include <stripes_solver.h>

StripesSolver::StripesSolver(const string & _puzzle_folder, int _stripes_n, int _samples_n) :
    puzzle_folder(_puzzle_folder),
    stripes_n(_stripes_n),
    sols_n(_samples_n),
    path_manager(_stripes_n, sols_n) {

    ocr = new tesseract::TessBaseAPI();
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

    ocr->SetVariable("language_model_penalty_non_freq_dict_word", "10");
    ocr->SetVariable("language_model_penalty_non_dict_word", "1");

#ifdef DEBUG
    FILE * file = fopen("tmp/variables.txt", "w");
    ocr->PrintVariables(file);
    fclose(file);
#endif

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

}

StripesSolver::~StripesSolver() {
    ocr->End();
}

void StripesSolver::save_result(const string & case_name) {

    const string result_folder = "data/results/"; 
    if (access(result_folder.c_str(), 0) == -1) {
        mkdir(result_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    string output_path = 
        result_folder + case_name + "_" + 
        to_string(static_cast<int>(metric_mode)) + "_" + 
        to_string(static_cast<int>(composition_mode));
        
    cv::imwrite(output_path + ".png", composition_img);
    cv::imwrite(output_path + "_seams.png", composition_img_seams);

}

bool StripesSolver::reassemble(Metric _metric_mode, Composition _composition_mode) {
    
    metric_mode = _metric_mode;
    composition_mode = _composition_mode;

    m_metric();

    vector< vector<int> > composition_orders;

    switch (composition_mode) {
        case Composition::GREEDY:
            cout << "[INFO] Composition: Greedy." << endl;
            composition_orders = reassemble_greedy();
            composition_order = composition_orders[0];
            break;
        case Composition::GREEDY_PROBABILITY:
            cout << "[INFO] Composition: Greedy in probability." << endl;
            reassemble_greedy_probability();
            break;
        default:
            return false;
    }

    composition_img = compose_img(composition_order);
    composition_img_seams = add_seams(composition_img, composition_order);

    return true;

}

cv::Mat StripesSolver::compose_img(const vector<int> & composition_order) {
    
    cv::Mat composition_img;

    for (int i = 0; i < composition_order.size(); i++) {
        composition_img = merge_imgs(composition_img, stripes[composition_order[i]]);
    }

    return composition_img;

}

cv::Mat StripesSolver::add_seams(const cv::Mat & img, const vector<int> & composition_order) {

    const cv::Scalar color_red(100, 100, 200);
    const cv::Scalar color_green(100, 200, 100);
    cv::Mat img_seams = img.clone();
    int col = stripes[composition_order[0]].cols;
    cv::Scalar seam_color;

    for (int i = 1; i < composition_order.size(); i++) {

        for (int j = 0; j < stripes_n; j++) {
            if (gt_order[j] != composition_order[i-1]) continue;
            if (j == stripes_n - 1 || gt_order[j+1] != composition_order[i]) {
                seam_color = color_red;
            } else {
                seam_color = color_green;
            }
            break;
        }

        cv::line(img_seams, cv::Point(col, 0), cv::Point(col, img.rows), seam_color);
        col += stripes[composition_order[i]].cols;

    }

    return img_seams;

}


double StripesSolver::m_metric_pixel(const cv::Mat & piece0, const cv::Mat & piece1) {

    assert(piece0.rows == piece1.rows);

    int x0 = piece0.cols - 1;
    int x1 = 0;

    double m_score = 0;
    double avg_pixel_color0 = 0;
    double avg_pixel_color1 = 0;
    for (int y = 0; y < piece0.rows; y++) {
        double avg_vec3b0 = avg_vec3b(piece0.at<cv::Vec3b>(y, x0));
        double avg_vec3b1 = avg_vec3b(piece1.at<cv::Vec3b>(y, x1));
        m_score += abs(avg_vec3b0 - avg_vec3b1);
        avg_pixel_color0 += avg_vec3b0;
        avg_pixel_color1 += avg_vec3b1;
    }

    avg_pixel_color0 /= piece0.rows;
    avg_pixel_color1 /= piece0.rows;
    m_score /= piece0.rows;

    if (max(avg_pixel_color0, avg_pixel_color1) < 15 ||
        min(avg_pixel_color0, avg_pixel_color1) > 240) {
        return -1;
    }
    return m_score;

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

void StripesSolver::m_metric() {

    // Compute matching score for each pair
    cout << "[INFO] Compute matching score for each pair." << endl;

    int filters_n = min(int(stripes_n * filter_rate), stripes_n - 1);

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

        pixel_graph = vector< vector<double> >(stripes_n, vector<double>(stripes_n));

        for (int i = 0; i < stripes_n; i++) {

            vector<StripePair> candidates;
            for (int j = 0; j < stripes_n; j++) {
                
                if (i == j) continue;

                double m_score = 0;
                switch (metric_mode) {
                    case Metric::PIXEL:
                        m_score = m_metric_pixel(stripes[i], stripes[j]);
                        break;
                    case Metric::WORD:
                        m_score = m_metric_word(stripes[i], stripes[j]);
                        break;
                    default:
                        break;
                }   

                pixel_graph[i][j] = m_score;
                candidates.push_back(StripePair(i, j, m_score, 1.0, true));

            }

            if (composition_mode == Composition::GREEDY_PROBABILITY) {
                
                sort(candidates.begin(), candidates.end());

                for (int j = 0; j < filters_n - 1; j++) {
                    int mid_idx = (stripes_n + j) >> 1;
                    if (candidates[j].m_score < eps) { // <= 0
                        candidates[j].ac_prob = 1;
                    } else {
                        double alpha = U_a * (candidates[mid_idx].m_score / candidates[j].m_score - 1);
                        candidates[j].ac_prob = exp(alpha) / (1 + exp(alpha));
                    }

                }

            }

            stripe_pairs.insert(
                stripe_pairs.end(),
                make_move_iterator(candidates.begin()),
                make_move_iterator(candidates.begin() + filters_n));

        }
    }

    sort(stripe_pairs.begin(), stripe_pairs.end());

#ifdef DEBUG
    for (const StripePair & sp : stripe_pairs) {
        cout << sp << endl;
    }
#endif

}

vector< vector<int> > StripesSolver::reassemble_greedy(bool probability_flag) {

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uniform_unit_dist(0, 1);

    vector<int> stripe_left(stripes_n, -1);
    vector<int> stripe_right(stripes_n, -1);

    int merged_cnt = 0;
    vector<StripePair> stripe_pairs_rest;

    for (const StripePair & sp: stripe_pairs) {

        if (abs(sp.m_score + 1) < eps) continue;

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

    vector< vector<int> > composition_orders;
    vector<bool> visited(stripes_n, false);

    for (int order_idx = 0; order_idx < stripes_n; order_idx++) {

        if (visited[order_idx]) continue;

        while (stripe_left[order_idx] != -1) {
            order_idx = stripe_left[order_idx];
        }

        vector<int> sol;
        sol.push_back(order_idx);
        visited[order_idx] = true;

        while (stripe_right[order_idx] != -1) {
            order_idx = stripe_right[order_idx];
            composition_img = merge_imgs(composition_img, stripes[order_idx]);
            sol.push_back(order_idx);
            visited[order_idx] = true;
        }

        composition_orders.push_back(move(sol));

    }

    return composition_orders;

}

void StripesSolver::reassemble_greedy_probability() {

    vector< vector<int> > candidate_sols;
    map< vector<int>, int > sols_cnt;

    while (candidate_sols.size() < sols_n) {
        
        printf("Run greedy algorithm:\t%d/%d.\tProb: True.\n", int(candidate_sols.size())+1, sols_n);

        vector< vector<int> > && composition_orders = reassemble_greedy(true);
        const vector<int> & sol = composition_orders[0];
        if (sol.size() != stripes_n) continue;

        sols_cnt[sol]++; 
        
        // Check if it appears at the first time;
        if (sols_cnt[sol] == 1) {
            candidate_sols.push_back(move(sol));
        }

    }

    int sol_idx = 0;

    for (const auto & sol: candidate_sols) {
        
        printf("Detect words on solution:\t%d/%d.\n", ++sol_idx, sols_n);
        
        cv::Mat composition_img = compose_img(sol);
        cv::Mat tmp_img = word_detection(composition_img, sol, sols_cnt[sol]);

#ifdef DEBUG
        tmp_img = add_seams(tmp_img, sol);
        cv::imwrite("tmp/sol_" + to_string(sol_idx) + ".png", tmp_img);
        // cv::imshow("Tmp img", tmp_img);
        // cv::waitKey();
#endif
    }

    path_manager.print_sol_paths();

    path_manager.build_path_graph();
    path_manager.print_path_graph();

    vector<StripePair> stripe_pairs_pixel = stripe_pairs;
    stripe_pairs = path_manager.build_stripe_pairs();
    vector< vector<int> > && composition_orders = reassemble_greedy(false);

    finetune_sols(composition_orders);

}

cv::Mat StripesSolver::word_detection(  const cv::Mat & img, 
                                        const vector<int> & sol,
                                        int sol_cnt) {

    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_WORD};
    const cv::Scalar color_blue(200, 0, 0);

    ocr->SetImage(img.data, img.cols, img.rows, 3, img.step);
    ocr->Recognize(0);
    
    tesseract::ResultIterator * word_iter = ocr->GetIterator();

    double m_metric_score = 0;
    cv::Mat img_bbox = img.clone();

    vector<int> sol_x;
    int cum_x = 0;
    for (int i = 0; i < sol.size(); i++) {
        sol_x.push_back(cum_x);
        cum_x += stripes[sol[i]].cols;
    }

    map< vector<int>, int > sol_words;

    if (word_iter != 0) {
        do {
            const float conf = word_iter->Confidence(tesseract_level);
            const string word = word_iter->GetUTF8Text(tesseract_level);
            if (word.length() < 2 || conf < conf_thres || !word_iter->WordIsFromDictionary()) continue;

            // Boundary cross constraint
            int x0, y0, x1, y1;
            word_iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            
            m_metric_score += conf * word.length();
            cv::rectangle(img_bbox, o_bbox, color_blue);

            int sol_path_st = upper_bound(sol_x.begin(), sol_x.end(), x0) - sol_x.begin() - 1;
            int sol_path_ed = lower_bound(sol_x.begin(), sol_x.end(), x1) - sol_x.begin();
            
            if (sol_path_ed - sol_path_st > 1) {
                sol_words[vector<int>(sol.begin()+sol_path_st, sol.begin()+sol_path_ed)] += sol_cnt;
            }

#ifdef DEBUG
            printf("word: '%s';  \tconf: %.2f; \tDict: %d; \tBoundingBox: %d,%d,%d,%d;\n",
                    word.c_str(), conf, word_iter->WordIsFromDictionary(), x0, y0, x1, y1);
            cout << endl;
#endif

        } while (word_iter->Next(tesseract_level));
    }

    path_manager.add_sol_words(sol_words, sol_cnt);

    return img_bbox;

}

void StripesSolver::finetune_sols(const vector< vector<int> > & composition_orders) {

    cout << "[INFO] Finetune composition orders." << endl;

    cout << endl << "Fragments:" << endl;
    for (const auto & sol: composition_orders) {
        cout << "[1]\t";
        for (const int & x: sol) cout << x << " ";
        cout << endl;
    }
    cout << endl;

    vector<int> in_nodes, out_nodes;

    for (const auto & sol: composition_orders) {
        // for (int i = 1; i < sol.size(); i++) {
            // cout << sol[i - 1] << "\t->\t" << sol[i] << "\tpixel-level score: " << pixel_graph[sol[i-1]][sol[i]] << endl;
        // }
        in_nodes.push_back(sol.front());
        out_nodes.push_back(sol.back());
    }

    vector<StripePair> end_pairs;
    for (int i = 0; i < out_nodes.size(); i++) {
        for (int j = 0; j < in_nodes.size(); j++) {
            if (i == j) continue;
            StripePair sp(out_nodes[i], in_nodes[j], pixel_graph[out_nodes[i]][in_nodes[j]], 1, true);
            end_pairs.push_back(sp);
        }
    }

    sort(end_pairs.begin(), end_pairs.end());

    for (const auto & sp: end_pairs) {
        cout << sp << endl;
    }

    composition_order = composition_orders[0];

}
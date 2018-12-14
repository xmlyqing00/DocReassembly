#include <debug_tool.h>

bool show_counter_example_pixel_metric( const cv::Mat & root_img,
                                        const cv::Mat & best_img,
                                        const cv::Mat & test_img,
                                        const double & m_score_best,
                                        cv::Mat & canvas,
                                        char operation) {
    
    double m_score_test = m_metric_pixel(root_img, test_img);
    cout << m_score_best << " " << m_score_test << endl;
    if (operation == '0') {
        if (m_score_best > m_score_test) return false;
    } else {
        if (m_score_best <= m_score_test) return false;
    }
    

    const int indication_width = 20;
    canvas = cv::Mat(   root_img.rows, 
                        root_img.cols * 4 + indication_width * 4 + 2, 
                        CV_8UC3, 
                        cv::Scalar(255, 255, 255));
    cv::Rect roi_rect;
    cv::Mat m_score_map(root_img.rows, indication_width, CV_32FC1);
    cv::Mat heat_map;

    // matching score between root and best.
    for (int y = 0; y < m_score_map.rows; y++) {
        int x0 = root_img.cols - 1;
        int x1 = 0;
        double m_score = diff_vec3b(root_img.at<cv::Vec3b>(y, x0), 
                                    best_img.at<cv::Vec3b>(y, x1));

        for (int x = 0; x < indication_width; x++) {
            m_score_map.at<float>(y, x) = (float)m_score;
        }
    }
    cv::normalize(m_score_map, heat_map, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::applyColorMap(heat_map, heat_map, cv::COLORMAP_JET);
    roi_rect = cv::Rect(root_img.cols * 2 + 1, 0, indication_width, root_img.rows);
    heat_map.copyTo(canvas(roi_rect));

    // draw abutting boundary
    cv::line(   canvas, 
                cv::Point(root_img.cols, 0),
                cv::Point(root_img.cols, root_img.rows),
                seam_color_green,
                1);

    // matching score between root and test.
    for (int y = 0; y < m_score_map.rows; y++) {
        int x0 = root_img.cols - 1;
        int x1 = 0;
        double m_score = diff_vec3b(root_img.at<cv::Vec3b>(y, x0), 
                                    test_img.at<cv::Vec3b>(y, x1));

        for (int x = 0; x < indication_width; x++) {
            m_score_map.at<float>(y, x) = (float)m_score;
        }
    }
    cv::normalize(m_score_map, heat_map, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::applyColorMap(heat_map, heat_map, cv::COLORMAP_JET);
    roi_rect = cv::Rect(root_img.cols * 4 + indication_width * 3 + 2, 0, indication_width, root_img.rows);
    heat_map.copyTo(canvas(roi_rect));

    // draw abutting boundary
    cv::line(   canvas, 
                cv::Point(root_img.cols * 3 + indication_width * 3 + 1, 0), 
                cv::Point(root_img.cols * 3 + indication_width * 3 + 1, root_img.rows),
                seam_color_red, 
                1);

    // root
    roi_rect = cv::Rect(0, 0, root_img.cols, root_img.rows);
    root_img.copyTo(canvas(roi_rect));
    
    // best
    roi_rect = cv::Rect(root_img.cols + 1, 0, root_img.cols, root_img.rows);
    best_img.copyTo(canvas(roi_rect));

    // root
    roi_rect = cv::Rect(root_img.cols * 2 + indication_width * 3 + 1, 0, root_img.cols, root_img.rows);
    root_img.copyTo(canvas(roi_rect));

    // test
    roi_rect = cv::Rect(root_img.cols * 3 + indication_width * 3 + 2, 0, root_img.cols, root_img.rows);
    test_img.copyTo(canvas(roi_rect));

    return true;
    
}

void find_counter_example_pixel_metric( const string & puzzle_folder,
                                        const string & case_name,
                                        int vertical_n,
                                        int operation) {

    ifstream fin(puzzle_folder + "order.txt", ios::in);
    if (!fin.is_open()) {
        cerr << "[ERRO] " << puzzle_folder + "order.txt" << " does not exist." << endl;
        exit(-1);
    }

    vector<int> order(vertical_n);
    for (int i = 0; i < vertical_n; i++) fin >> order[i];
    fin.close();

    for (int root_id = 0; root_id < vertical_n; root_id++) {

        cout << endl << "Input piece root id: \t" << root_id << endl;

        cv::Mat root_img = cv::imread(puzzle_folder + to_string(root_id) + ".png");

        int best_id = -1;
        for (int i = 0; i < vertical_n - 1; i++) {
            if (order[i] == root_id) {
                best_id = order[i + 1];
                break;
            }
        }

        if (best_id == -1) {
            cout << "[INFO] Best matching does not exist." << endl;
            continue;
        }

        cv::Mat best_img = cv::imread(puzzle_folder + to_string(best_id) + ".png");

        double m_score_best = m_metric_pixel(root_img, best_img);
        cv::Mat canvas;

        for (int i = 0; i < vertical_n; i++) {
            
            if (i == root_id) continue;
            if (i == best_id) continue;
            
            cv::Mat test_img = cv::imread(puzzle_folder + to_string(i) + ".png");
            
            bool found = show_counter_example_pixel_metric( root_img, 
                                                            best_img, 
                                                            test_img,
                                                            m_score_best,
                                                            canvas,
                                                            operation);

            if (found) {
                printf("Root: %d,\t Best: %d,\t Test: %d\n", root_id, best_id, i);
                const string img_path = output_folder + "pixel_" +
                                        case_name + "_" + 
                                        to_string(root_id) + "_" + 
                                        to_string(best_id) + "_" +
                                        to_string(i) + ".png";
                cv::imwrite(img_path, canvas);
                cv::imshow("canvas", canvas);
                cv::waitKey(0);
            }

        }

    }
}

bool show_counter_example_ocr_char_metric(  const cv::Mat & root_img,
                                            const cv::Mat & test_img,
                                            tesseract::TessBaseAPI * ocr,
                                            vector<cv::Mat> & out_imgs) {

    const int seam_x = root_img.cols;
    const tesseract::PageIteratorLevel tesseract_level {tesseract::RIL_SYMBOL};
    out_imgs.clear();

    cv::Mat && merged_img = merge_imgs(root_img, test_img);

    ocr->SetImage(merged_img.data, merged_img.cols, merged_img.rows, 3, merged_img.step);
    ocr->Recognize(0);

    tesseract::ResultIterator * iter = ocr->GetIterator();

    // draw canvas
    cv::Mat canvas(root_img.rows, root_img.cols + test_img.cols + 1, CV_8UC3);
    cv::Rect roi_rect;

    // draw abutting boundary
    cv::line(   canvas, 
                cv::Point(root_img.cols, 0), 
                cv::Point(root_img.cols, root_img.rows),
                seam_color_red, 
                1);

    // root
    roi_rect = cv::Rect(0, 0, root_img.cols, root_img.rows);
    root_img.copyTo(canvas(roi_rect));
    
    // test
    roi_rect = cv::Rect(root_img.cols + 1, 0, root_img.cols, root_img.rows);
    test_img.copyTo(canvas(roi_rect));

    if (iter != 0) {
        do {

            // Boundary cross constraint
            int x0, y0, x1, y1;
            iter->BoundingBox(tesseract_level, &x0, &y0, &x1, &y1);
            const cv::Rect o_bbox(x0, y0, x1 - x0, y1 - y0);
            if (!cross_seam(o_bbox, seam_x)) continue;
            const int w = 3;
            cv::Rect e_bbox(o_bbox.x - w, o_bbox.y - w, o_bbox.width + w * 2, o_bbox.height + w * 2);

            const string symbol = iter->GetUTF8Text(tesseract_level);
            const float conf = iter->Confidence(tesseract_level);
            printf("%s\t%.1lf\t", symbol.c_str(), conf);
            cout << e_bbox << endl;

            cv::Mat tmp_img = canvas.clone();
            cv::rectangle(tmp_img, e_bbox, cv::Scalar(200, 0, 0), 1);
            out_imgs.push_back(tmp_img.clone());
            
        } while (iter->Next(tesseract_level));
    }

    return out_imgs.size();

}

void find_counter_example_ocr_char_metric(  const string & puzzle_folder,
                                            const string & case_name,
                                            int vertical_n) {
    
    ifstream fin(puzzle_folder + "order.txt", ios::in);
    if (!fin.is_open()) {
        cerr << "[ERRO] " << puzzle_folder + "order.txt" << " does not exist." << endl;
        exit(-1);
    }

    vector<int> order(vertical_n);
    for (int i = 0; i < vertical_n; i++) fin >> order[i];
    fin.close();

    tesseract::TessBaseAPI * ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY)) {
        cerr << "Could not initialize tesseract." << endl;
        exit(-1);
    }

    for (int root_id = 0; root_id < vertical_n; root_id++) {

        cout << endl << "Input piece root id: \t" << root_id << endl;

        cv::Mat root_img = cv::imread(puzzle_folder + to_string(root_id) + ".png");

        int best_id = -1;
        for (int i = 0; i < vertical_n - 1; i++) {
            if (order[i] == root_id) {
                best_id = order[i + 1];
                break;
            }
        }

        vector<cv::Mat> out_imgs;

        for (int i = 0; i < vertical_n; i++) {
            
            if (i == root_id) continue;
            if (i == best_id) continue;

            cv::Mat test_img = cv::imread(puzzle_folder + to_string(i) + ".png");
            
            bool found = show_counter_example_ocr_char_metric(  root_img, 
                                                                test_img,
                                                                ocr,
                                                                out_imgs);

            if (found) {
                printf("Root: %d,\t Test: %d\n", root_id, i);
                const string img_path_prefix =  output_folder + "ocr_" + 
                                                case_name + "_" + 
                                                to_string(root_id) + "_" + 
                                                to_string(i) + "_";

                for (int j = 0; j < (int)out_imgs.size(); j++) {
                    string img_path = img_path_prefix + to_string(j) + ".png";
                    cv::imwrite(img_path, out_imgs[j]);
                    cv::imshow("canvas", out_imgs[j]);
                    cv::waitKey(0);
                }
                
                
            }

        }

    }

    ocr->End();

}

void add_seams(const string & puzzle_name) {

    cv::Mat sol_img = cv::imread("data/results/" + puzzle_name + "_GA_sol.png");

    const string puzzle_folder = "data/stripes/" + puzzle_name + "/";
    ifstream fin(puzzle_folder + "/order.txt");
    vector<int> gt_order;
    if (!fin.is_open()) {
        cerr << "[ERRO] " << puzzle_name + "/order.txt" << " does not exist." << endl;
        exit(-1);
    }

    while (!fin.eof()) {
        int x;
        fin >> x;
        gt_order.push_back(x);
    }
    fin.close();

    int n = gt_order.size();
    vector<int> sol_order(n);
    for (int i = 0; i < n; i++) {
        cv::Mat stripe_img = cv::imread(puzzle_folder + to_string(i) + ".png");
        for (int j = 0; j < n; j++) {
            cv::Rect roi_rect(j * stripe_img.cols, 0, stripe_img.cols, stripe_img.rows);
            cv::Mat diff_img = sol_img(roi_rect) - stripe_img;
            cv::Scalar diff_mean = cv::mean(diff_mean);
            if (diff_mean[0] + diff_mean[1] + diff_mean[2] < 10) {
                cout << i << " " << j << " " << diff_mean << endl;
            }
        }
    }


}

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0_noise_5";
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    int vertical_n = 20;
    DebugType debug_type = DebugType::Pixel;
    string operation = "doc3_noise1_40";
    // 0 | 1 for DebugType Pixel
    // doc3_noise1_40 for DebugType AddSeam

    // Parse command line parameters
    const string opt_str = "t:n:sd:o:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't':
                case_name = string(optarg);
                break;
            case 'n':
                vertical_n = atoi(optarg);
                break;
            case 's':
                puzzle_type = PuzzleType::SQUARES;
                break;
            case 'd':
                debug_type = static_cast<DebugType>(atoi(optarg));
                break;
            case 'o':
                operation = string(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "operation:           \t" << operation << endl;

    string puzzle_folder;
    if (puzzle_type == PuzzleType::STRIPES) {
        puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
    } else {
        puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
    }

    if (access(output_folder.c_str(), 0) == -1) {
        mkdir(output_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    cout << "Debug type:          \t";
    switch (debug_type) {
        case DebugType::Pixel:
            cout << "Pixel" << endl;
            find_counter_example_pixel_metric(puzzle_folder, case_name, vertical_n, operation[0]);
            break;
        case DebugType::OCRChar:
            cout << "OCR Char" << endl;
            find_counter_example_ocr_char_metric(puzzle_folder, case_name, vertical_n);
            break;
        case DebugType::AddSeam:
            cout << "Add Seams" << endl;
            add_seams(operation);
            break;
        default:
            cout << "Unknown" << endl;
            break;
    }

    return 0;
}
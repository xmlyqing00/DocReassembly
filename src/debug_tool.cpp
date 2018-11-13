#include <debug_tool.h>

bool counter_exmple_pixel_metric(   const cv::Mat & root_img,
                                    const cv::Mat & best_img,
                                    const cv::Mat & test_img,
                                    const double & m_score_best,
                                    cv::Mat & canvas) {
    
    double m_score_test = m_metric_pixel(root_img, test_img);
    cout << m_score_best << " " << m_score_test << endl;
    if (m_score_best > m_score_test) return false;

    const int indication_width = 30;
    const double scale = 50;
    canvas = cv::Mat(   root_img.rows, 
                        root_img.cols * 4 + indication_width * 4, 
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
    roi_rect = cv::Rect(root_img.cols, 0, indication_width, root_img.rows);
    heat_map.copyTo(canvas(roi_rect));

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
    roi_rect = cv::Rect(root_img.cols * 3 + indication_width * 3, 0, indication_width, root_img.rows);
    heat_map.copyTo(canvas(roi_rect));

    // root
    roi_rect = cv::Rect(0, 0, root_img.cols, root_img.rows);
    root_img.copyTo(canvas(roi_rect));
    
    // best
    roi_rect = cv::Rect(root_img.cols + indication_width, 0, root_img.cols, root_img.rows);
    best_img.copyTo(canvas(roi_rect));

    // root
    roi_rect = cv::Rect(root_img.cols * 2 + indication_width * 3, 0, root_img.cols, root_img.rows);
    root_img.copyTo(canvas(roi_rect));

    // test
    roi_rect = cv::Rect(root_img.cols * 3 + indication_width * 4, 0, root_img.cols, root_img.rows);
    test_img.copyTo(canvas(roi_rect));

    return true;
    
}


int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0_noise_5";
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    int vertical_n = 20;
    string model_path = "data/models/";
    DebugType debug_type = DebugType::Pixel;

    // Parse command line parameters
    const string opt_str = "t:n:m:sd:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't':
                case_name = string(optarg);
                break;
            case 'n':
                vertical_n = atoi(optarg);
                break;
            case 'm':
                model_path = string(optarg);
                break;
            case 's':
                puzzle_type = PuzzleType::SQUARES;
                break;
            case 'd':
                debug_type = static_cast<DebugType>(atoi(optarg));
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "OCR model path:      \t" << model_path << endl;
    cout << "Debug type:          \t" << (debug_type == DebugType::Pixel ? "Pixel": "Unknown") << endl;

    string puzzle_folder;
    if (puzzle_type == PuzzleType::STRIPES) {
        puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
    } else {
        puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
    }

    if (debug_type == DebugType::Pixel) {

        ifstream fin(puzzle_folder + "order.txt", ios::in);
        if (!fin.is_open()) {
            cerr << "[ERRO] " << puzzle_folder + "order.txt" << " does not exist." << endl;
            exit(-1);
        }

        vector<int> order(vertical_n);
        for (int i = 0; i < vertical_n; i++) fin >> order[i];
        fin.close();

        if (access(output_folder.c_str(), 0) == -1) {
            mkdir(output_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
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
                
                bool found = counter_exmple_pixel_metric(   root_img, 
                                                            best_img, 
                                                            test_img,
                                                            m_score_best,
                                                            canvas);

                if (found) {
                    printf("Root: %d,\t Best: %d,\t Test: %d\n", root_id, best_id, i);
                    const string img_path = output_folder + case_name + "_" + 
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

    return 0;
}
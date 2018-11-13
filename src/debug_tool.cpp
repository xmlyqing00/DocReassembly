#include <debug_tool.h>

bool counter_exmple_pixel_metric(   const cv::Mat & root_img,
                                    const cv::Mat & best_img,
                                    const cv::Mat & test_img,
                                    const double & m_score_best) {
    
    double m_score_test = m_metric_pixel(root_img, test_img);
    if (m_score_best > m_score_test) return false;

    cv::Mat canvas = cv::Mat::ones(root_img.rows, root_img.cols * 6, CV_8UC3);
    cv::imshow("canvas", canvas);
    cv::waitKey(0);

    return true;
    
}


int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0";
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    int vertical_n = 4;
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

        int root_id;
        cin >> root_id;
        cv::Mat root_img = cv::imread(puzzle_folder + to_string(root_id) + ".png");

        ifstream fin(puzzle_folder + "order.txt", ios::in);
        if (!fin.is_open()) {
            cerr << "[ERRO] " << puzzle_folder + "order.txt" << " does not exist." << endl;
            exit(-1);
        }

        vector<int> order(vertical_n);
        int best_id = -1;
        for (int i = 0; i < vertical_n; i++) {
            fin >> order[i];
            if (order[i - 1] == root_id) {
                best_id = order[i];
            }
        }
        fin.close();

        if (best_id == -1) {
            cout << "[INFO] Best matching does not exist." << endl;
            exit(0);
        }

        cv::Mat best_img = cv::imread(puzzle_folder + to_string(best_id) + ".png");

        double m_score_best = m_metric_pixel(root_img, best_img);

        for (int i = 0; i < vertical_n; i++) {
            
            if (i == root_id) continue;
            if (i == best_id) continue;
            
            cv::Mat test_img = cv::imread(puzzle_folder + to_string(i) + ".png");
            
            bool found = counter_exmple_pixel_metric(   root_img, 
                                                        best_img, 
                                                        test_img,
                                                        m_score_best);

            if (found) {
                printf("Root: %d,\t Best: %d,\t Test: %d\n", root_id, best_id, i);
            }

        }

    }

    return 0;
}
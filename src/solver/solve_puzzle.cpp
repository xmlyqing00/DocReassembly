#include <solve_puzzle.h>

void solve_stripes( const string & stripes_folder, 
                    const string & case_name,
                    int vertical_n,
                    int samples_n,
                    StripesSolver::Metric metric_mode,
                    StripesSolver::Composition composition_mode,
                    bool benchmark_flag,
                    bool real_flag) {
    
    StripesSolver stripes_solver(stripes_folder, vertical_n, samples_n, real_flag);
    stripes_solver.reassemble(metric_mode, composition_mode, case_name, benchmark_flag);

#ifdef DEBUG

    cv::imshow("Composition Image", stripes_solver.composition_img);
    if (!stripes_solver.composition_img_seams.empty()) {
        cv::imshow("Composition Image Seams", stripes_solver.composition_img_seams);
    }
    if (!stripes_solver.composition_img_bar.empty()) {
        cv::imshow("Composition Image Bar", stripes_solver.composition_img_bar);
    }
    
    cv::waitKey();
#endif
}

void solve_squares (const string & squares_folder, 
                    const string & case_name,
                    int vertical_n) {

    const string puzzle_size_file_path = squares_folder + "puzzle_size.txt";
    cv::Size puzzle_size;
    ifstream fin(puzzle_size_file_path, ios::in);
    if (!fin.is_open()) {
        cerr << "[ERRO] " << puzzle_size_file_path << " does not exist." << endl;
        exit(-1);
    }
    fin >> puzzle_size.width;
    fin >> puzzle_size.height;
    fin.close();
    int squares_n = puzzle_size.width * puzzle_size.height;
    cout << "Squares number:      \t" << squares_n << endl;
    cout << endl;

    SquaresSolver squares_solver(puzzle_size);

    cout << "[INFO] Import squares." << endl;

    for (int i = 0; i < squares_n; i++) {
        const string square_img_path = squares_folder + to_string(i) + ".png";
        cv::Mat square_img = cv::imread(square_img_path);
        if (square_img.empty()) {
            cerr << "[ERR] Square img does not exist." << endl;
            exit(-1); 
        }
        squares_solver.push(square_img);
    }

    squares_solver.reassemble();


}

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0";
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    int vertical_n = 4;
    int samples_n = 10;
    StripesSolver::Composition composition_mode = StripesSolver::GREEDY;
    StripesSolver::Metric metric_mode = StripesSolver::PIXEL;
    bool benchmark_flag = false;
    bool real_flag = false;

    // Parse command line parameters
    const string opt_str = "t:n:Sc:m:s:br";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't':
                case_name = string(optarg);
                break;
            case 'n': 
                vertical_n = atoi(optarg);
                break;
            case 'c': 
                composition_mode = static_cast<StripesSolver::Composition>(atoi(optarg));
                break;
            case 'S': 
                puzzle_type = PuzzleType::SQUARES;
                break;
            case 'm':
                metric_mode = static_cast<StripesSolver::Metric>(atoi(optarg));
                break;
            case 's':
                samples_n = atoi(optarg);
                break;
            case 'b':
                benchmark_flag = true;
                break;
            case 'r':
                real_flag = true;
                break;
            default:
                cerr << "[ ERR] Unknon options " << opt << endl;
                exit(-1);
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    const string metric_mode_str = 
        metric_mode == StripesSolver::Metric::PIXEL ? "Pixel" :
        metric_mode == StripesSolver::Metric::CHAR ? "Char" :
        "Word";
    const string comp_mode_str = 
        composition_mode == StripesSolver::Composition::GREEDY ? "Greedy":
        composition_mode == StripesSolver::Composition::GCOM ? "GCOM":
        composition_mode == StripesSolver::Composition::GREEDY_GCOM ? "Greedy + GCOM (for test)":
        composition_mode == StripesSolver::Composition::GT ? "Groundtruth":
        "User";


    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "Metric mode:         \t" << metric_mode_str << endl;
    cout << "Composition mode:    \t" << comp_mode_str << endl;
    cout << "Real-world case:     \t" << real_flag << endl;
    if (composition_mode == StripesSolver::Composition::GCOM || composition_mode == StripesSolver::Composition::GREEDY_GCOM) {
        cout << "Samples times:       \t" << samples_n << endl;
    }

    time_t start_time = time(0);

    // Import stripes
    if (puzzle_type == PuzzleType::STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder, case_name, vertical_n, samples_n, metric_mode, composition_mode, benchmark_flag, real_flag);

    } else {

        const string puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_squares(puzzle_folder, case_name, vertical_n);

    }

    time_t end_time = time(0);
    double total_time = difftime(end_time, start_time);

    cout << "Time used: " << total_time << " s" << endl << endl;

    return 0;
}

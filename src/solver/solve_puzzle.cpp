#include <solve_puzzle.h>

void solve_stripes( const string & stripes_folder, 
                    const string & case_name,
                    int vertical_n,
                    int samples_n,
                    StripesSolver::Metric metric_mode,
                    StripesSolver::Composition composition_mode) {
    
    StripesSolver stripes_solver(stripes_folder, vertical_n, samples_n);
    stripes_solver.reassemble(metric_mode, composition_mode);
    stripes_solver.save_result(case_name);

#ifdef DEBUG
    for (int idx: stripes_solver.composition_order) {
        cout << idx << endl;
    }

    cv::imshow("Composition Image", stripes_solver.composition_img);
    cv::imshow("Composition Image Seams", stripes_solver.composition_img_seams);
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

    // Parse command line parameters
    const string opt_str = "t:n:Sc:m:s:";
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
            default:
                cerr << "[ ERR] Unknon options " << opt << endl;
                exit(-1);
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    const string metric_mode_str = 
        metric_mode == StripesSolver::Metric::PIXEL ? "Pixel" :
        metric_mode == StripesSolver::Metric::WORD ? "Word" :
        "Compability Evaluator";

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "Metric mode:         \t" << metric_mode_str << endl;
    cout << "Samples times:       \t" << samples_n << endl;
    cout << "Composition mode:    \t" << (composition_mode == StripesSolver::Composition::GREEDY ? "Greedy" : "Greedy Probability") << endl;

    clock_t start_time = clock();

    // Import stripes
    if (puzzle_type == PuzzleType::STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder, case_name, vertical_n, samples_n, metric_mode, composition_mode);

    } else {

        const string puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_squares(puzzle_folder, case_name, vertical_n);

    }

    clock_t end_time = clock();

    cout << "Time used: " << double(end_time - start_time) / CLOCKS_PER_SEC << " s" << endl << endl;

    return 0;
}

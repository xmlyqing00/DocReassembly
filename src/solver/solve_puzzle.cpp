#include <solve_puzzle.h>

void solve_stripes( const string & stripes_folder, 
                    const string & model_path,
                    const string & case_name,
                    const int vertical_n) {
    
    StripesSolver::Composition comp_mode = StripesSolver::GREEDY;
    StripesSolver::Metric metric_mode = StripesSolver::COMP_EVA;

    cout << "Composition mode:    \t" << comp_mode << endl;
    cout << "Metric mode:         \t" << metric_mode << endl;
    cout << endl;

    StripesSolver stripes_solver(model_path);

    cout << "[INFO] Import stripes." << endl;
    for (int i = 0; i < vertical_n; i++) {
        const string stripe_img_path = stripes_folder + to_string(i) + ".png";
        cv::Mat stripe_img = cv::imread(stripe_img_path);
        if (stripe_img.empty()) {
            cerr << "[ERR] Stripe img does not exist." << endl;
            exit(-1); 
        }
        stripes_solver.push(stripe_img);
    }

    stripes_solver.reassemble(metric_mode, comp_mode);
    stripes_solver.save_result(case_name);

    for (const int idx: stripes_solver.comp_idx) {
        cout << idx << endl;
    }

    cv::imshow("comp_img", stripes_solver.comp_img);
    cv::waitKey();

}

void solve_squares (const string & squares_folder, 
                    const string & model_path,
                    const string & case_name,
                    const int vertical_n) {

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

    SquaresSolver squares_solver(model_path, puzzle_size);

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
    string model_path = "data/models/";

    cv::Mat arial_img = cv::imread("data/symbols/arial.png");
    cv::imshow("arial_img", arial_img);
    cv::waitKey();

    // Parse command line parameters
    const string opt_str = "t:T:n:N:m:M:sS";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't': case 'T':
                case_name = string(optarg);
                break;
            case 'n': case 'N':
                vertical_n = atoi(optarg);
                break;
            case 'm': case 'M':
                model_path = string(optarg);
                break;
            case 's': case 'S':
                puzzle_type = PuzzleType::SQUARES;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "OCR model path:      \t" << model_path << endl;

    // Import stripes
    if (puzzle_type == PuzzleType::STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder, model_path, case_name, vertical_n);

    } else {

        const string puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_squares(puzzle_folder, model_path, case_name, vertical_n);

    }

    return 0;
}

#include <solve_puzzle.h>


void solve_stripes( const string & stripes_folder, 
                    const string & model_path,
                    const string & case_name,
                    const int vertical_n) {
    
    StripesSolver::Composition comp_mode = StripesSolver::GREEDY;
    StripesSolver::Metric metric_mode = StripesSolver::PIXEL;

    cout << "Composition mode:    \t" << comp_mode << endl;
    cout << "Metric mode:         \t" << metric_mode << endl;

    StripesSolver stripes(model_path);

    for (int i = 0; i < vertical_n; i++) {
        const string stripe_img_path = stripes_folder + to_string(i) + ".png";
        cv::Mat stripe_img = cv::imread(stripe_img_path);
        stripes.push(stripe_img);
    }

    stripes.reassemble(metric_mode, comp_mode);
    stripes.save_result(case_name);

    for (const int idx: stripes.comp_idx) {
        cout << idx << endl;
    }

    cv::imshow("comp_img", stripes.comp_img);
    cv::waitKey();

}

void solve_squares (const string & squares_folder, 
                    const string & model_path,
                    const string & case_name,
                    const int vertical_n) {

    const string puzzle_size_file_path = squares_folder + "puzzle_size.txt";
    FILE * file = fopen(squares_folder.c_str(), "r");

}

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    PuzzleType puzzle_type = STRIPES;
    int vertical_n = 4;
    string model_path = "data/models/";

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
                puzzle_type = SQUARES;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type ? "Squares": "Stripes") << endl;
    cout << "OCR model path:      \t" << model_path << endl;

    // Import stripes
    if (puzzle_type == STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder, model_path, case_name, vertical_n);

    } else {

        const string puzzle_folder = "data/squares/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_squares(puzzle_folder, model_path, case_name, vertical_n);

    }
    

   

    return 0;
}

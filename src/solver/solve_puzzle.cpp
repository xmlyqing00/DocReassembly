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
    const string opt_str = "t:n:c:m:s:br";
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

    // Import stripes
    if (puzzle_type == PuzzleType::STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder, case_name, vertical_n, samples_n, metric_mode, composition_mode, benchmark_flag, real_flag);

    }

    return 0;
}

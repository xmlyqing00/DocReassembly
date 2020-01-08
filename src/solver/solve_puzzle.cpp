#include <solve_puzzle.h>

// Default parameters
string case_name = "doc0";
PuzzleType puzzle_type = PuzzleType::STRIPES;
int vertical_n = 4;
int samples_n = 10;
StripesSolver::Composition composition_mode = StripesSolver::GREEDY;
StripesSolver::Metric metric_mode = StripesSolver::PIXEL;
bool benchmark_flag = false;
bool real_flag = false;

// -- Default params for synthetic cases
double word_conf_thres = 70;
double lambda0 = 0.3;
double lambda1 = 0.5;
double U_a = 2;
double filter_rate = 0.7;
int candidate_factor = 4;

void solve_stripes( const string & stripes_folder) {
    
    StripesSolver stripes_solver(stripes_folder, vertical_n, samples_n, real_flag, word_conf_thres, lambda0, lambda1, U_a, filter_rate, candidate_factor);
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

void print_help() {
    cout << 
        "--test <str>                   The test case to evaluate.\n"
        "--num <int>                    The number of stripes.\n"
        "--comp <int>                   The composition type: 0 Greedy, 1 GCOM,\n"
        "                               2 Greedy and GCOM, 3 Groundtruth,\n"
        "                               4 user defined.\n"
        "--metric <int>                 The similarity metric type: 0 pixel-level,\n"
        "                               1 character-level, 2 word-level.\n"
        "--samples <int>                When GCOM is set, it defines how many\n"
        "                               sequences are sampled for word-level OCR. Note\n"
        "                               that high reassembly score requires enough samples.\n"
        "                               For 20 stripes, we recommend 150 samples.\n"
        "                               For 30 stripes, we recommend 300 samples.\n"
        "                               For 40 stripes, we recommend 1000 samples.\n"
        "                               For 60 stripes, we recommend 8000 samples.\n"
        "--benchmark                    Flag. Whether to write results in files.\n"
        "--real                         Flag. Whether the test case is in real-world.\n";
        " "
        "--word_conf_thres <float>      OCR score threshold [0-100]. Default 70.\n"
        "--lambda0 <float>              Balance char-level and pixel-level metrics in Eq. 5.\n"
        "                               [0-1]. Default 0.3.\n"
        "--lambda1 <float>              Balance word-level and low-level metrics in Eq. 13.\n"
        "                               [0-1]. Default: 0.5.\n"
        "--u_a <float>                  Scale factor in Eq. 9. [0-100]. Default: 2.\n"
        "--filter_rate <float>          Filter out stripe pairs that have low-level scores. \n"
        "                               [0-1]. Default: 0.7.\n"
        "--candidate_factor <int>       Seq len = Stripe num / candidate_factor. Default 4.\n";
}

int main(int argc, char ** argv) {

    // Parse command line parameters
    const string short_opts = "t:n:c:m:s:brh1:2:3:4:5:6:";
    const option long_opts[] = {
        {"test", required_argument, nullptr, 't'},
        {"num", required_argument, nullptr, 'n'},
        {"comp", required_argument, nullptr, 'c'},
        {"metric", required_argument, nullptr, 'm'},
        {"samples", required_argument, nullptr, 's'},
        {"benchmark", no_argument, nullptr, 'b'},
        {"real", no_argument, nullptr, 'r'},
        {"help", no_argument, nullptr, 'h'},
        {"word_conf_thres", required_argument, nullptr, 1},
        {"lambda0", required_argument, nullptr, 2},
        {"lambda1", required_argument, nullptr, 3},
        {"u_a", required_argument, nullptr, 4},
        {"filter_rate", required_argument, nullptr, 5},
        {"candidate_factor", required_argument, nullptr, 6},
        {nullptr, no_argument, nullptr, 0}
    };
    int opt_id;

    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts.c_str(), long_opts, &opt_id);

        if (opt == -1) break;

        switch (opt) {
            
            case 0:
                printf("option %s", long_opts[opt_id].name);
                if (optarg) printf(" with arg %s", optarg);
                printf("\n");
                break;
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
            case 'h':
                print_help();
                exit(0);

            case 1:
                word_conf_thres = atof(optarg);
                break;
            case 2:
                lambda0 = atof(optarg);
                break;
            case 3:
                lambda1 = atof(optarg);
                break;
            case 4:
                U_a = atof(optarg);
                break;
            case 5:
                filter_rate = atof(optarg);
                break;
            case 6:
                candidate_factor = atoi(optarg);
                break;
            default:
                print_help();
                cerr << "[ ERR] Unknon options " << opt << endl;
                exit(-1);
        }
        
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
    cout << "word_conf_thres:     \t" << word_conf_thres << endl;
    cout << "lambda0:             \t" << lambda0 << endl;
    cout << "lambda1:             \t" << lambda1 << endl;
    cout << "u_a:                 \t" << U_a << endl;
    cout << "filter_rate:         \t" << filter_rate << endl;
    cout << "candidate_factor:    \t" << candidate_factor << endl;

    // Import stripes
    if (puzzle_type == PuzzleType::STRIPES) {

        const string puzzle_folder = "data/stripes/" + case_name + "_" + to_string(vertical_n) + "/";
        solve_stripes(puzzle_folder);

    }

    return 0;
}

#include <generate_puzzle.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    int vertical_n = 4;
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    bool updown_flag = false;

    // Parse command line parameters
    const string opt_str = "t:n:U";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't':
                case_name = string(optarg);
                break;
            case 'n':
                vertical_n = atoi(optarg);
                break;
            case 'U':
                updown_flag = true;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Up down flag:        \t" << updown_flag << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;

    // Generate new puzzle
    string puzzle_folder;
    if (puzzle_type == PuzzleType::STRIPES) {
        puzzle_folder = "data/stripes/";
    } else {
        puzzle_folder = "data/squares/";
    }

    if (access(puzzle_folder.c_str(), 0) == -1) {
        mkdir(puzzle_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    if (updown_flag) {
        puzzle_folder += case_name + "_" + to_string(vertical_n * 2) + "/";    
    } else {
        puzzle_folder += case_name + "_" + to_string(vertical_n) + "/";
    }

    const string gt_folder = "data/gt/";
    const string gt_img_path = gt_folder + case_name + ".png";
    cv::Mat gt_img = cv::imread(gt_img_path);

    if (gt_img.empty()) {
        cerr << "[ERR] No such test case: " << gt_img_path << endl;
        exit(-1);
    }

    if (puzzle_type == PuzzleType::STRIPES) {
        StripesGenerator stripes_generator(gt_img_path, vertical_n, updown_flag);
        stripes_generator.save_puzzle(puzzle_folder);
    }
        
    return 0;
    
}
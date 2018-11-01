#include <generate_puzzle.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    int vertical_n = 4;
    bool regenerate_flag = false;
    PuzzleType puzzle_type = STRIPES;

    // Parse command line parameters
    const string opt_str = "t:T:n:N:gGsS";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't': case 'T':
                case_name = string(optarg);
                break;
            case 'n': case 'N':
                vertical_n = atoi(optarg);
                break;
            case 'g': case 'G':
                regenerate_flag = true;
                break;
            case 's': case 'S':
                puzzle_type = SQUARES;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Re-generate puzzle:  \t" << boolalpha << regenerate_flag << endl;
    cout << "Puzzle type:         \t" << (puzzle_type ? "Squares": "Stripes") << endl;

    // Generate new puzzle
    string puzzle_folder;
    if (puzzle_type == STRIPES) {
        puzzle_folder = "data/stripes/";
    } else {
        puzzle_folder = "data/squares/";
    }

    if (access(puzzle_folder.c_str(), 0) == -1) {
        mkdir(puzzle_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    puzzle_folder += case_name + "_" + to_string(vertical_n) + "/";

    if (regenerate_flag || access(puzzle_folder.c_str(), 0) == -1) {

        const string gt_folder = "data/gt/";
        const string gt_img_path = gt_folder + case_name + ".png";
        cv::Mat gt_img = cv::imread(gt_img_path);

        if (gt_img.empty()) {
            cerr << "[ERR] No such test case: " << gt_img_path << endl;
            exit(-1);
        }

        if (puzzle_type == STRIPES) {
            StripesGenerator stripes_generator(gt_img_path, vertical_n);
            stripes_generator.save_puzzle(puzzle_folder);
        } else {
            SquaresGenerator squares_generator(gt_img_path, vertical_n);
            squares_generator.save_puzzle(puzzle_folder);
        }
        
    }

    return 0;
    
}
#include <debug_tool.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0";
    PuzzleType puzzle_type = PuzzleType::STRIPES;
    int vertical_n = 4;
    string model_path = "data/models/";

    // Parse command line parameters
    const string opt_str = "t:n:m:s";
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
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Vertical cut num:    \t" << vertical_n << endl;
    cout << "Puzzle type:         \t" << (puzzle_type == PuzzleType::SQUARES ? "Squares": "Stripes") << endl;
    cout << "OCR model path:      \t" << model_path << endl;


    return 0;
}
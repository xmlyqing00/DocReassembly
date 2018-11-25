#include <build_dataset.h>

int main(int argc, char ** argv) {

    // Default parameters
    string font_name = "arial";
    int symbol_n = 62;
    bool extract_symbols_flag = false;

    // Parse command line parameters
    const string opt_str = "t:n:e";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 'f':
                font_name = string(optarg);
                break;
            case 'n':
                symbol_n = atoi(optarg);
                break;
            case 'e':
                extract_symbols_flag = true;
                break;
                
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Font name:           \t" << font_name << endl;
    cout << "Symbols num:         \t" << symbol_n << endl;
    cout << "Extract symbols:     \t" << boolalpha << extract_symbols_flag << endl;

    cv::Mat arial_img = cv::imread("data/symbols/arial.png");
    cv::imshow("arial_img", arial_img);
    cv::waitKey();

    return 0;
}
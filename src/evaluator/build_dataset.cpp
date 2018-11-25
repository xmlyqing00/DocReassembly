#include <build_dataset.h>

bool valid_symbol(const char ch) {
    if (ch >= '0' && ch <= '9') return true;
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    return false;
}

void extract_symbols(const string & font_name) {

    const string symbol_folder = "data/symbols/";
    cv::Mat font_img = cv::imread(symbol_folder + font_name + ".png");
#ifdef DEBUG
    cv::imshow("font img", font_img);
#endif

    ifstream in_file(symbol_folder + font_name + ".box");
    string line;
    int symbol_idx = 0;

    while (getline(in_file, line)) {

        if (!valid_symbol(line[0])) continue;
        istringstream iss(line);

        char symbol; 
        int x0, y0, x1, y1;
        iss >> symbol >> x0 >> y1 >> x1 >> y0;

        y0 = font_img.rows - y0;
        y1 = font_img.rows - y1;
        cv::Rect roi_rect(x0, y0, x1 - x0, y1 - y0);

        cv::Mat symbol_img = font_img(roi_rect);
#ifdef DEBUG
        cout << line << endl;
        cv::imshow("symbol", symbol_img);
        cv::waitKey();
#endif

        const string symbol_name = symbol_folder + font_name + "_" + to_string(symbol_idx++) + ".png";
        cv::imwrite(symbol_name, symbol_img);

    }

    in_file.close();
    
}

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

    if (extract_symbols_flag) {
        extract_symbols(font_name);
    }
    

    return 0;
}
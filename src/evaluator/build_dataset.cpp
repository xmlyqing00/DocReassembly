#include <build_dataset.h>

bool valid_symbol(char ch) {
    if (ch >= '0' && ch <= '9') return true;
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    return false;
}

void extract_symbols(const string & font_name) {
    
    cout << "Extract symbols." << endl;

    cv::Mat font_img = cv::imread(symbol_folder + font_name + ".png");
#ifdef DEBUG
    cv::imshow("font img", font_img);
#endif

    ifstream in_file(symbol_folder + font_name + ".box", ios::in);
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

        const string symbols_name = symbol_folder + font_name + "_" + to_string(symbol_idx++) + ".png";
        cv::imwrite(symbols_name, symbol_img);

    }

    in_file.close();

    const string symbols_name = symbol_folder + font_name + "_" + to_string(symbol_idx++) + ".png";
    cv::Mat blank_img(48, 48, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::imwrite(symbols_name, blank_img);
    
}

void partition_dataset() {

    cout << "Partition dataset into train and test." << endl;
    ifstream in_file(data_root + "cp_dataset.txt", ios::out);
    if (!in_file) {
        cerr << "File " << data_root + "training.txt" << " doesn't exist." << endl;
        exit(-1);
    }
    
    int data_n;
    in_file >> data_n;
    vector< pair<int, char> > data(data_n, make_pair(0, '?'));
    for (int i = 0; i < data_n; i++) {
        in_file >> i >> data[i].first >> data[i].second;
    }
    in_file.close();

    vector<int> access_idx(data_n);
    default_random_engine rand_engine(time(0));

    iota(access_idx.begin(), access_idx.end(), 0);
    shuffle(access_idx.begin(), access_idx.end(), rand_engine);

    int train_n = int(data_n * partition_rate);
    int test_n = data_n - train_n;

    cout << "Training dataset number: " << train_n << endl;
    cout << "Testing dataset number: " << test_n << endl;

    ofstream out_file(data_root + "cp_dataset_train.txt", ios::out);
    out_file << train_n << endl;
    for (int i = 0; i < train_n; i++) {
        int idx = access_idx[i];
        out_file << idx << " " << data[idx].first << " " << data[idx].second << endl;
    }
    out_file.close();

    out_file = ofstream(data_root + "cp_dataset_test.txt", ios::out);
    out_file << test_n << endl;
    for (int i = 0; i < test_n; i++) {
        int idx = access_idx[train_n + i];
        out_file << idx << " " << data[idx].first << " " << data[idx].second << endl;
    }
    out_file.close();

}

void generate_dataset(  const string & font_name, 
                        int symbols_n, 
                        int output_w) {
    
    cout << "Generate compatibility dataset." << endl;

    vector<char> symbols;
    for (int i = 0; i < 10; i++) symbols.push_back('0' + i);
    for (int i = 0; i < 26; i++) symbols.push_back('A' + i);
    for (int i = 0; i < 26; i++) symbols.push_back('a' + i);
    symbols.push_back('.');

    vector<cv::Mat> symbol_imgs;
    for (int i = 0; i < symbols_n; i++) {
        cv::Mat symbol_img = cv::imread(symbol_folder + font_name + "_" + to_string(i) + ".png");
        symbol_imgs.push_back(move(symbol_img));
    }

    if (access(dataset_folder.c_str(), 0) == -1) {
        mkdir(dataset_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    int data_idx = 0;
    int data_total = symbols_n * (symbols_n - 1 + symbols_n - 1) * 2;
    ofstream out_file(data_root + "cp_dataset.txt");
    out_file << data_total << endl;

    cout << "Dataset number: " << data_total << endl;

    int canvas_w = 256;
    int canvas_w_half = canvas_w >> 1;
    int canvas_h_half = canvas_w >> 1;

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_int_distribution<int> uni_int(0, symbols_n - 1);

    for (int i = 0; i < symbols_n; i++) {

        // Left half symbol
        int s0_w_half = symbol_imgs[i].cols >> 1;
        int s0_h_half = symbol_imgs[i].rows >> 1;
        cv::Rect s0_roi_rect(0, 0, s0_w_half, symbol_imgs[i].rows);

        for (int j = 0; j < symbols_n; j++) {

            // Right half symbol
            int s1_w_half = symbol_imgs[j].cols >> 1;
            int s1_h_half = symbol_imgs[j].rows >> 1;
            cv::Rect s1_roi_rect(s1_w_half, 0, s1_w_half, symbol_imgs[j].rows);

            // Copy left and right half symbols to canvas
            cv::Mat canvas(canvas_w, canvas_w, CV_8UC3, cv::Scalar(255, 255, 255));
            cv::Rect canvas_roi_rect(   canvas_w_half - s0_w_half, canvas_h_half - s0_h_half,
                                        s0_w_half, symbol_imgs[i].rows);
            symbol_imgs[i](s0_roi_rect).copyTo(canvas(canvas_roi_rect));

            canvas_roi_rect = cv::Rect( canvas_w_half, canvas_h_half - s1_h_half,
                                        s1_w_half, symbol_imgs[j].rows);
            symbol_imgs[j](s1_roi_rect).copyTo(canvas(canvas_roi_rect));

            // Compability flag
            int comp = (i == j);
            int noise_n = (comp == 1) ? symbols_n - 1 : 1;
            
            for (int t = 0; t < noise_n; t++) {

                cv::Mat store_canvas = canvas.clone();

                // Add left noisy symbol
                int rand_symbol_idx = uni_int(rand_engine);
                canvas_roi_rect = cv::Rect( canvas_w_half - s0_w_half - symbol_imgs[rand_symbol_idx].cols,
                                            canvas_h_half - symbol_imgs[rand_symbol_idx].rows / 2,
                                            symbol_imgs[rand_symbol_idx].cols,
                                            symbol_imgs[rand_symbol_idx].rows);
                symbol_imgs[rand_symbol_idx].copyTo(canvas(canvas_roi_rect));

                // Add right noisy symbol
                rand_symbol_idx = uni_int(rand_engine);
                canvas_roi_rect = cv::Rect( canvas_w_half + s0_w_half,
                                            canvas_h_half - symbol_imgs[rand_symbol_idx].rows / 2,
                                            symbol_imgs[rand_symbol_idx].cols,
                                            symbol_imgs[rand_symbol_idx].rows);
                symbol_imgs[rand_symbol_idx].copyTo(canvas(canvas_roi_rect));

                // Save canvas to training data
                canvas_roi_rect = cv::Rect( canvas_w_half - output_w / 2, 
                                            canvas_h_half - output_w / 2,
                                            output_w,
                                            output_w);
                string output_path = dataset_folder + to_string(data_idx) + ".png";
                cv::imwrite(output_path, canvas(canvas_roi_rect));
                out_file << data_idx++ << " " << comp << " " << ((comp == 1) ? symbols[i] : '?') << endl;

                // Resize canvas to 50% and save it to training data
                cv::resize(canvas, canvas, cv::Size(), 0.5, 0.5);
                canvas_roi_rect = cv::Rect( canvas_w_half / 2 - output_w / 2, 
                                            canvas_h_half / 2 - output_w / 2,
                                            output_w,
                                            output_w);
                output_path = dataset_folder + to_string(data_idx) + ".png";
                cv::imwrite(output_path, canvas(canvas_roi_rect));
                out_file << data_idx++ << " " << comp << " " << ((comp == 1) ? symbols[i] : '?') << endl;

#ifdef DEBUG
                cv::imshow("canvas", canvas);
                cv::waitKey();
#endif

                canvas = store_canvas;

            }

        }
    }

    out_file.close();

    partition_dataset();

}

int main(int argc, char ** argv) {

    // Default parameters
    string font_name = "arial";
    int symbols_n = 63;
    int output_w = 64;
    BuildType build_type = BuildType::ALL;

    // Parse command line parameters
    const string opt_str = "t:n:b:w:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 'f':
                font_name = string(optarg);
                break;
            case 'n':
                symbols_n = atoi(optarg);
                break;
            case 'b':
                build_type = static_cast<BuildType>(atoi(optarg));
                break;
            case 'w':
                output_w = atoi(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Font name:           \t" << font_name << endl;
    cout << "Symbols num:         \t" << symbols_n << endl;
    cout << "Training data size:  \t" << output_w << " x " << output_w << endl;
    cout << "Build dataset type:  \t" << static_cast<int>(build_type) << endl;
    cout << endl;

    switch (build_type) {

        case BuildType::EXTRACT_SYMBOLS:
            extract_symbols(font_name);
            break;
        case BuildType::TRAINING: 
            generate_dataset(font_name, symbols_n, output_w);
            break;
        case BuildType::ALL:
            extract_symbols(font_name);
            generate_dataset(font_name, symbols_n, output_w);
            break;

    }
    

    return 0;
}
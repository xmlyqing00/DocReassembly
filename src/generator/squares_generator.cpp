#include <squares_generator.h>

SquaresGenerator::SquaresGenerator(const string & img_path, int vertical_n) {
    
    ori_img = cv::imread(img_path);
    ori_img_size = ori_img.size();
    
    square_size.width = ori_img_size.width / vertical_n;
    square_size.height = square_size.width;
    puzzle_size.width = vertical_n;
    puzzle_size.height = ori_img_size.height / square_size.height;

    squares_n = puzzle_size.width * puzzle_size.height;

    seg_squares();

}

cv::Mat SquaresGenerator::get_puzzle_img(int gap=5) {

    cv::Mat puzzle_img = cv::Mat::zeros(ori_img_size.height + (puzzle_size.height - 1) * gap, 
                                        ori_img_size.width + (puzzle_size.width - 1) * gap, 
                                        CV_8UC3);
    
    int square_idx = 0;
    for (int i = 0; i < puzzle_size.height; i++) {
        for (int j = 0; j < puzzle_size.width; j++) {
            cv::Rect roi_rect(  square_size.width * j + gap * j,
                                square_size.height * i + gap * i,
                                square_size.width,
                                square_size.height);
            squares[square_idx++].copyTo(puzzle_img(roi_rect));
        }
    }

    // cv::imshow("puzzle_img", puzzle_img);
    // cv::waitKey();

    return puzzle_img;

}

bool SquaresGenerator::seg_squares() {

    for (int i = 0; i < puzzle_size.height; i++) {
        for (int j = 0; j < puzzle_size.width; j++) {
            cv::Rect ori_rect(  j * square_size.width, 
                                i * square_size.height, 
                                square_size.width, 
                                square_size.height);
            // cout << ori_rect << endl;
            cv::Mat square_img = ori_img(ori_rect);
            squares.push_back(square_img.clone());       
        }
    }

    access_idx = vector<int>(squares_n);
    default_random_engine rand_engine(time(0));

    iota(access_idx.begin(), access_idx.end(), 0);
    shuffle(access_idx.begin(), access_idx.end(), rand_engine);

    return true;

}

bool SquaresGenerator::save_puzzle(const string & output_folder) {
    
    if (access(output_folder.c_str(), 0) == -1) {
        mkdir(output_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    for (int i = 0; i < squares_n; i++) {
        cv::imwrite(output_folder + to_string(i) + ".png", squares[access_idx[i]]);
    }

    vector<int> gt_order(squares_n);
    for (int i = 0; i < squares_n; i++) {
        gt_order[access_idx[i]] = i;
    }

    const string puzzle_size_file_path = output_folder + "puzzle_size.txt";
    ofstream fout(puzzle_size_file_path, ios::out);
    fout << puzzle_size.width << endl;
    fout << puzzle_size.height << endl;
    fout.close();

    const string order_file_path = output_folder + "order.txt";
    fout = ofstream(order_file_path, ios::out);
    
    for (const int & i: gt_order) {
        fout << i << endl;
    }
    fout.close();

    cv::Mat && puzzle_img = get_puzzle_img(3);
    cv::imwrite(output_folder + "puzzle_img.png", puzzle_img);

    cout << "Squares saved path: \t" << output_folder << endl;

    return true;

}
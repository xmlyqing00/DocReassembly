#include <stripes_generator.h>

StripesGenerator::StripesGenerator(const string & img_path, int _stripes_n, bool _updown_flag) :
    updown_flag(_updown_flag) {
    
    ori_img = cv::imread(img_path);
    ori_img_size = ori_img.size();
    stripes_n = _stripes_n;

    seg_stripes();

}

cv::Mat StripesGenerator::get_puzzle_img(int gap=5) {

    cv::Size img_size(ori_img_size.width + (stripes_n - 1) * gap, ori_img.rows);
    if (updown_flag) {
        img_size.width <<= 1;
    }
    cv::Mat puzzle_img = cv::Mat::zeros(img_size, CV_8UC3);
    
    int puzzle_img_x = 0;
    for (const int idx: access_idx) {
        cv::Rect roi(puzzle_img_x, 0, stripes[idx].cols, stripes[idx].rows);
        stripes[idx].copyTo(puzzle_img(roi));

        if (gap == 0 && puzzle_img_x > 0) {
            cv::line(puzzle_img, cv::Point(puzzle_img_x, 0), cv::Point(puzzle_img_x, puzzle_img.rows), cv::Scalar(0, 0, 0));
        }
        puzzle_img_x += stripes[idx].cols + gap;
        
    }

    // cv::imwrite("tmp/puzzle_img.png", puzzle_img);
    // cv::imshow("puzzle_img", puzzle_img);
    // cv::waitKey();

    return puzzle_img;

}

bool StripesGenerator::seg_stripes() {

    int seg_step = int((double)ori_img_size.width / stripes_n);

    int seg_st = 0;
    for (int i = 0; i < stripes_n; i++) {

        int seg_ed = seg_st + seg_step;
        if (i == stripes_n - 1) {
            seg_ed = ori_img_size.width;
        }

        cv::Mat stripe = ori_img(cv::Rect(seg_st, 0, seg_ed - seg_st, ori_img_size.height));
        stripes.push_back(stripe.clone());

        seg_st = seg_ed;
        
    }

    if (updown_flag) {
        int n = stripes.size();
        for (int i = n - 1; i >= 0; i--) {
            cv::Mat flipped_img = stripes[i].clone();
            cv::flip(flipped_img, flipped_img, -1);
            stripes.push_back(move(flipped_img));
        }
    }

    access_idx.clear();
    default_random_engine rand_engine(time(0));

    for (int i = 0; i < stripes_n; i++) {
        access_idx.push_back(i);
    }

    if (updown_flag) {
        for (int i = stripes_n - 1; i >= 0; i--) {
            access_idx.push_back(i);
        }
        stripes_n <<= 1;
    }
    
    
    iota(access_idx.begin(), access_idx.end(), 0);
    shuffle(access_idx.begin(), access_idx.end(), rand_engine);

    return true;

}

bool StripesGenerator::save_puzzle(const string & output_folder) {
    
    if (access(output_folder.c_str(), 0) == -1) {
        mkdir(output_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    for (int i = 0; i < stripes_n; i++) {
        cv::imwrite(output_folder + to_string(i) + ".png", stripes[access_idx[i]]);
    }

    vector<int> gt_order(stripes_n);
    for (int i = 0; i < stripes_n; i++) {
        gt_order[access_idx[i]] = i;
    }
    const string order_file_path = output_folder + "order.txt";
    ofstream fout(order_file_path, ios::out);
    for (const int & i: gt_order) {
        fout << i << endl;
    }
    fout.close();

    cv::Mat && puzzle_img = get_puzzle_img(0);
    cv::imwrite(output_folder + "puzzle_img.png", puzzle_img);

    cout << "Stripes saved path:\t" << output_folder << endl;

    return true;

}
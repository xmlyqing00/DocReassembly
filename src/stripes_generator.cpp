#include <stripes_generator.h>

StripesGenerator::StripesGenerator(string img_path, int _stripes_n) {
    
    ori_img = cv::imread(img_path);
    ori_img_size = ori_img.size();
    stripes_n = _stripes_n;

    seg_stripes();

}

void StripesGenerator::show_whole_stripes(bool rearrange) {

    int gap = 5;
    cv::Mat whole_stripes = cv::Mat::zeros(ori_img_size.height, ori_img_size.width + (stripes_n - 1) * gap, CV_8UC3);
    vector<int> access_idx(stripes_n);
    iota(access_idx.begin(), access_idx.end(), 0);

    if (rearrange) {
        default_random_engine rand_engine(time(0));
        shuffle(access_idx.begin(), access_idx.end(), rand_engine);
    }
    
    int whole_stripes_x = 0;
    for (const int idx: access_idx) {
        cv::Rect roi(whole_stripes_x, 0, stripes[idx].cols, stripes[idx].rows);
        stripes[idx].copyTo(whole_stripes(roi));
        whole_stripes_x += stripes[idx].cols + gap;
    }

    // cv::imwrite("tmp/whole_stripes.png", whole_stripes);
    // cv::imshow("whole_stripes", whole_stripes);
    // cv::waitKey();

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

    return true;

}

bool StripesGenerator::save_stripes(const string & output_folder) {

    string output_path = "data/stripes/";
    if (access(output_path.c_str(), 0) == -1) {
        int create_flag = mkdir(output_path.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
        if (create_flag != 0) {
            cerr << "Create Stripes folder failed." << endl;
            return false;
        }
    }
    
    output_path += output_folder;
    if (output_path.back() != '/') output_path += '/';

    if (access(output_path.c_str(), 0) == -1) {

        cout << "Stripes folder does not exist." << endl;

        int create_flag = mkdir(output_path.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
        if (create_flag == 0) {
            cout << "Create stripes folder." << endl;
        } else {
            cerr << "Create Stripes folder failed." << endl;
            return false;
        }

    }

    for (int i = 0; i < stripes_n; i++) {
        cv::imwrite(output_path + to_string(i) + ".png", stripes[i]);
    }

    cout << "Save tripes to " << output_path << endl;

    return true;

}
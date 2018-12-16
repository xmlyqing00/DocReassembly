#include <add_noise.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "doc0";
    int noise_level = 5;
    int dense_level = 20;

    // Parse command line parameters
    const string opt_str = "t:l:d:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't':
                case_name = string(optarg);
                break;
            case 'l':
                noise_level = atoi(optarg);
                break;
            case 'd':
                dense_level = atoi(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    string out_name = case_name + "_noise_" + to_string(dense_level);

    cout << "Test case name:      \t" << case_name << endl;
    cout << "Saved name:          \t" << out_name << endl;
    cout << "Noise level [1-100]: \t" << noise_level << endl;
    cout << "Dense level [1-1000]:\t" << dense_level << endl;
    
    const string gt_folder = "data/gt/";
    const string in_img_path = gt_folder + case_name + ".png";
    const string out_img_path = gt_folder + out_name + ".png";
    
    cv::Mat in_img = cv::imread(in_img_path);

    random_device rand_device;
    default_random_engine rand_engine(rand_device());
    uniform_real_distribution<double> uni_dist(1 - (double)noise_level / 100, 1);
    uniform_int_distribution<int> uni_dense(0, 1000);

    for (int y = 0; y < in_img.rows; y++) {
        for (int x = 0; x < in_img.cols; x++) {

            int dense_test = rand_engine() % 1000;
            // dense_test = int(abs(sin(x * y)) * dense_test);
            if (dense_test > dense_level) continue;

            cv::Vec3b color = in_img.at<cv::Vec3b>(y, x);
            double noise_alpha = uni_dist(rand_engine);
            
            for (int k = 0; k < 3; k++) {
                color[k] = (uchar)(color[k] * noise_alpha);
            }
            in_img.at<cv::Vec3b>(y, x) = color;
        }
    }

    cv::imwrite(out_img_path, in_img);

    cout << "[INFO] Saved noisy image." << endl;

    return 0;

}
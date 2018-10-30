#include <generate_stripes.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    int stripes_n = 4;
    bool generate_flag = false;

    // Parse command line parameters
    const string opt_str = "t:T:n:N:gG";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't': case 'T':
                case_name = string(optarg);
                break;
            case 'n': case 'N':
                stripes_n = atoi(optarg);
                break;
            case 'g': case 'G':
                generate_flag = true;
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Case name:\t\t" << case_name << endl;
    cout << "Stripes num:\t\t" << stripes_n << endl;
    cout << "Generate stripes:\t" << boolalpha << generate_flag << endl;
    cout << endl;

    // Generate new stripes
    const string stripes_folder = "data/stripes/" + case_name + "_" + to_string(stripes_n) + "/";

    if (generate_flag || access(stripes_folder.c_str(), 0) == -1) {
        const string gt_folder = "data/gt/";
        const string gt_img_path = gt_folder + case_name + ".png";
        cv::Mat gt_img = cv::imread(gt_img_path);

        StripesGenerator stripes_generator(gt_img_path, stripes_n);
        stripes_generator.save_stripes(stripes_folder);
    }

    return 0;
    
}
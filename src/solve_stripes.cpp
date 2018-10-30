#include <solve_stripes.h>

int main(int argc, char ** argv) {

    // Default parameters
    string case_name = "test0";
    int stripes_n = 4;
    Stripes::Composition comp_mod = Stripes::GREEDY;
    string model_path = "data/models/";

    // Parse command line parameters
    const string opt_str = "t:T:n:N:m:M:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 't': case 'T':
                case_name = string(optarg);
                break;
            case 'n': case 'N':
                stripes_n = atoi(optarg);
                break;
            case 'm': case 'M':
                model_path = string(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Case name:\t\t" << case_name << endl;
    cout << "Stripes num:\t\t" << stripes_n << endl;
    cout << "OCR model path:\t\t" << model_path << endl;
    cout << endl;

    // Import stripes
    const string stripes_folder = "data/stripes/" + case_name + "_" + to_string(stripes_n) + "/";
    Stripes stripes(model_path);

    for (int i = 0; i < stripes_n; i++) {
        const string stripe_img_path = stripes_folder + to_string(i) + ".png";
        cv::Mat stripe_img = cv::imread(stripe_img_path);
        stripes.push(stripe_img);
    }

    stripes.reassemble(comp_mod);
    stripes.save_result(case_name);
    for (const int idx: stripes.comp_idx) {
        cout << idx << endl;
    }

    cv::imshow("comp_img", stripes.comp_img);
    cv::waitKey();

    return 0;
}

#include <test_realdata.h>

int main(int argc, char ** argv) {

    // Default parameters
    int data_n = 10;

    // Parse command line parameters
    const string opt_str = "n:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 'n':
                data_n = atoi(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Total data num:      \t" << data_n << endl;
    cout << endl;


    const string model_path = saved_model_folder + "best.pt";
    if (access(model_path.c_str(), 0) == -1) {
        cerr << "Model file: " << model_path << " does not exist!" << endl;
        exit(-1);
    }
    serialize::InputArchive input_archive;
    input_archive.load_from(model_path);

    CompatibilityNet cp_net;
    cp_net.load(input_archive);

    vector<char> symbols;
    for (int i = 0; i < 10; i++) symbols.push_back('0' + i);
    for (int i = 0; i < 26; i++) symbols.push_back('A' + i);
    for (int i = 0; i < 26; i++) symbols.push_back('a' + i);
    symbols.push_back('.');
    symbols.push_back('?');

    DeviceType device_type;
    if (cuda::is_available()) {
        cout << "CUDA available! Training on GPU" << endl;
        device_type = kCUDA;
    } else {
        cout << "Training on CPU" << endl;
        device_type = kCPU;
    }
    Device device(device_type);
    cp_net.to(device);

    for (int i = 0; i < data_n; i++) {

        cv::Mat img = cv::imread(realdata_folder + to_string(i) + ".png");
        cv::resize(img, img, cp_net_imgsize);
        Tensor img_tensor = torch::from_blob(img.data, {img.rows, img.cols, 3}, kByte);
        img_tensor = img_tensor.permute({2, 0, 1}).toType(kFloat32).div_(255).unsqueeze(0);
        img_tensor = img_tensor.to(device);
        
        Tensor output = cp_net.forward(img_tensor);
        int class_idx = output.argmax(1).item<int>();

        cout << output << endl;
        cout << "pred " << class_idx << " " << symbols[class_idx] << endl;

        cv::imshow("img", img);
        cv::waitKey();

    }

    return 0;

}

#include <compatibility_dataset.h>

CompatibilityDataset::CompatibilityDataset(Mode _mode) {
    
    cout << "Load " << (_mode == Mode::kTrain ? "TRAIN" : "TEST") << " dataset" << endl;

    mode = _mode;
    string list_path = data_root;
    if (mode == Mode::kTrain) {
        list_path += "cp_dataset_train.txt";
    } else {
        list_path += "cp_dataset_test.txt";
    }

    ifstream in_file(list_path, ios::in);
    if (!in_file) {
        cerr << "File " << list_path << " doesn't exist." << endl;
        in_file.close();
        exit(-1);
    }

    in_file >> data_n;
    data = vector< pair<int, char> >(data_n);
    for (int i = 0; i < data_n; i++) {
        int tmp;
        in_file >> data[i].first >> tmp >> data[i].second;
    }

    in_file.close();

    // Init symbol2class
    int idx = 0;
    memset(symbol2class, 255, sizeof(symbol2class));
    for (int i = 0; i < 10; i++) symbol2class['0' + i] = idx++;
    for (int i = 0; i < 26; i++) symbol2class['A' + i] = idx++;
    for (int i = 0; i < 26; i++) symbol2class['a' + i] = idx++;
    symbol2class['.'] = idx++;
    symbol2class['?'] = idx++;

}

data::Example<> CompatibilityDataset::get(size_t index) {

    // index = 15345;
    // cout << index << " " << data[index].first << endl;
    
    cv::Mat img = cv::imread(dataset_folder + to_string(data[index].first) + ".png");
    // cv::Mat img = cv::Mat(64, 64, CV_8UC3, cv::Scalar(255, 255, 255));
    img.convertTo(img, CV_32FC3, 1.0 / 255);
    
    // Tensor img_tensor = torch::from_blob(img.data, {img_size.height, img_size.width, 3}, dtype(kFloat));
    // img_tensor = img_tensor.permute({2, 0, 1});
    // cout << img_tensor << endl;
    // cv::imshow("tmp", img);
    Tensor img_tensor = torch::rand({3, 64, 64});

    int class_idx = symbol2class[data[index].second];
    Tensor target = torch::tensor(class_idx, dtype(kInt64));
    
    return {img_tensor, target};
}

torch::optional<size_t> CompatibilityDataset::size() const {
    return data_n;
}

bool CompatibilityDataset::is_train() const noexcept {
    return true;
}
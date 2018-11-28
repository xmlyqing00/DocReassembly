#ifndef COMPATIBILITY_DATASET_H
#define COMPATIBILITY_DATASET_H

#include <istream>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <opencv2/opencv.hpp>
#include <torch/torch.h>

using namespace std;
using namespace torch;

class CompatibilityDataset: 
    public data::datasets::Dataset<CompatibilityDataset> {

public:
    enum class Mode { kTrain, kTest };
    Mode mode;
    int data_n {0};
    vector< pair<int, char> > data;

    explicit CompatibilityDataset(Mode mode);

    data::Example<> get(size_t index);
    torch::optional<size_t> size() const;
    bool is_train() const noexcept;

private:
    const string data_root {"data/"};
    const string dataset_folder {"data/cp_dataset/"};

    const cv::Size img_size {64, 64};
    int symbol2class[256];
};

#endif
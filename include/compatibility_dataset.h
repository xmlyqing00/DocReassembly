#ifndef COMPATIBILITY_DATASET_H
#define COMPATIBILITY_DATASET_H

#include <string>
#include <vector>
#include <torch/torch.h>

using namespace std;
using namespace torch;

class CompatibilityDataset: 
    public data::datasets::Dataset<CompatibilityDataset> {

public:
    enum class Mode { kTrain, kTest };

    explicit CompatibilityDataset(const string & dataset_folder);

    data::Example<> get(size_t index) override;
    torch::optional<size_t> size() const override;

};

#endif
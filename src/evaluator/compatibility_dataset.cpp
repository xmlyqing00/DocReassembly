#include <compatibility_dataset.h>

CompatibilityDataset::CompatibilityDataset(const string & dataset_folder) {
    cout << dataset_folder << endl;
}

data::Example<> CompatibilityDataset::get(size_t index) {
    Tensor img = torch::rand({3, 64, 64});
    Tensor target = torch::randint(0, 64, {1}, dtype(kInt64));
    return {img, target};
}

torch::optional<size_t> CompatibilityDataset::size() const {
    return 1000;
}

bool CompatibilityDataset::is_train() const noexcept {
    return true;
}
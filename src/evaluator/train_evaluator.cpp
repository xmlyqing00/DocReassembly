#include <train_evaluator.h>

int main() {

    int batch_size = 64;
    double lr = 1e-2;
    double momentum = 1e-4;

    DeviceType device_type;
    if (cuda::is_available()) {
        cout << "CUDA available! Training on GPU" << endl;
        device_type = kCUDA;
    } else {
        cout << "Training on CPU" << endl;
        device_type = kCPU;
    }
    Device device(device_type);

    CompatibilityNet comp_net;
    comp_net.to(device);

    torch::optim::SGD optimizer(
        comp_net.parameters(),
        torch::optim::SGDOptions(lr).momentum(momentum));

    Tensor tensor = torch::rand({1, 3, 64, 64});
    Tensor y = comp_net.forward(tensor);
    cout << y << endl;

    return 0;
    
}
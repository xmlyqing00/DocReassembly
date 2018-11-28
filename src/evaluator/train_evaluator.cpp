#include <train_evaluator.h>

template<typename DataLoader>
void train( int epoch, 
            CompatibilityNet & comp_net, 
            DataLoader & data_loader,
            optim::SGD & optimizer,
            const Device & device) {
    
    comp_net.train();
    int batch_idx = 0;

    for (auto & batch: data_loader) {
        
        Tensor data = batch.data.to(device);
        Tensor target = batch.target.to(device);
        target = squeeze(target, /*dim*/1);

        Tensor output = comp_net.forward(data);
        Tensor loss = nll_loss(output, target);
        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (batch_idx++ % 10 == 0) {
            printf("Train epoch: %d [%4d/%4d]\tLoss: %.3f\n",
                epoch, int(batch_idx * batch.data.size(0)), 10,
                loss.template item<float>());
        }

    }

}

int main() {

    int epochs = 100;
    int batch_size = 1;
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

    auto train_loader = data::make_data_loader(
        CompatibilityDataset(dataset_path).map(torch::data::transforms::Stack<>()),
        batch_size
    );

    auto test_loader = data::make_data_loader(
        CompatibilityDataset(dataset_path).map(torch::data::transforms::Stack<>()), 
        batch_size
    );

    torch::optim::SGD optimizer(
        comp_net.parameters(),
        torch::optim::SGDOptions(lr).momentum(momentum)
    );

    for (int epoch = 1; epoch <= epochs; epoch++) {
        train(epoch, comp_net, *train_loader, optimizer, device);
    }

    return 0;
    
}

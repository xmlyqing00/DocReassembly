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
        cout << target << endl;
        cout << output << endl;
        Tensor loss = nll_loss(output, target);
        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (batch_idx++ % 1 == 0) {
            printf("Train [epoch: %3d | %5d]\tLoss: %.3f\n",
                epoch, int(batch_idx * batch.data.size(0)),
                loss.template item<float>());
        }

        if (batch_idx == 10) break;

    }

}

int main(int argc, char ** argv) {

    // Default parameters
    int epochs = 100;
    int batch_size = 128;
    double lr = 1e-3;
    double momentum = 0.9;

    // Parse command line parameters
    const string opt_str = "e:b:l:m:";
    int opt = getopt(argc, argv, opt_str.c_str());

    while (opt != -1) {
        switch (opt) {
            case 'e':
                epochs = atoi(optarg);
                break;
            case 'b':
                batch_size = atoi(optarg);
                break;
            case 'l':
                lr = atof(optarg);
                break;
            case 'm':
                momentum = atof(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Total epochs:        \t" << epochs << endl;
    cout << "Batch size:          \t" << batch_size << endl;
    cout << "Learning rate:       \t" << lr << endl;
    cout << "Momentum:            \t" << momentum << endl;
    cout << endl;

    DeviceType device_type;
    int workers;
    if (cuda::is_available()) {
        cout << "CUDA available! Training on GPU" << endl;
        device_type = kCUDA;
        workers = 4;
    } else {
        cout << "Training on CPU" << endl;
        device_type = kCPU;
        workers = 0;
    }
    Device device(device_type);

    CompatibilityNet comp_net;
    comp_net.to(device);

    data::DataLoaderOptions dataloader_options;
    dataloader_options.batch_size(batch_size);
    dataloader_options.workers(workers);

    auto train_loader = data::make_data_loader(
        CompatibilityDataset(CompatibilityDataset::Mode::kTrain).
            map(data::transforms::Stack<>()),
        dataloader_options
    );

    auto test_loader = data::make_data_loader(
        CompatibilityDataset(CompatibilityDataset::Mode::kTest).
            map(data::transforms::Stack<>()),
        dataloader_options
    );

    optim::SGD optimizer(
        comp_net.parameters(),
        optim::SGDOptions(lr).momentum(momentum)
    );

    for (int epoch = 1; epoch <= epochs; epoch++) {
        train(epoch, comp_net, *train_loader, optimizer, device);
    }

    return 0;
    
}

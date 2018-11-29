#include <train_evaluator.h>

template<typename DataLoader, typename Optimizer>
void train( int epoch, 
            CompatibilityNet & comp_net, 
            DataLoader & data_loader,
            Optimizer & optimizer,
            const Device & device) {
    
    comp_net.train();
    int batch_idx = 0;

    for (auto & batch: data_loader) {
        
        Tensor data = batch.data.to(device);
        Tensor target = batch.target.to(device);
        target = squeeze(target, /*dim*/1);

        Tensor output = comp_net.forward(data);
        
        Tensor loss = nll_loss(output, target, symbols_w);
        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (batch_idx++ % 10 == 0) {
            printf("Train [epoch: %3d | %5d]\tLoss: %.6f\n",
                epoch, int(batch_idx * batch.data.size(0)),
                loss.template item<float>());
        }

    }

    const string model_path = saved_model_folder + to_string(epoch) + ".pt";
    serialize::OutputArchive output_archive;
    comp_net.save(output_archive);
    output_archive.save_to(model_path);
    cout << "Saved model at " << model_path << endl;

}

template<typename DataLoader>
void test(  CompatibilityNet & comp_net, 
            DataLoader & data_loader,
            const Device & device) {
    
    comp_net.eval();
    NoGradGuard no_grad;

    float test_loss = 0;
    int correct_n = 0;
    int total_n = 0;

    for (const auto & batch: data_loader) {
        
        Tensor data = batch.data.to(device);
        Tensor target = batch.target.to(device);
        target = squeeze(target, /*dim*/1);

        Tensor output = comp_net.forward(data);

        test_loss += nll_loss(output, target, symbols_w, Reduction::Sum).template item<float>();
        auto pred = output.argmax(1);
        correct_n += pred.eq(target).sum().template item<int64_t>();
        total_n += batch.data.size(0);

    }

    test_loss /= total_n;
    float avg_correct = (float)correct_n / total_n;

    printf("Test\tAvg Correct: %.2f\tAvg Loss: %.6f\n", avg_correct, test_loss);

}

int main(int argc, char ** argv) {

    // Default parameters
    int epochs = 500;
    int batch_size = 128;
    double lr = 1e-2;
    double alpha = 0.9;

    // Parse command line parameters
    const string opt_str = "e:b:l:a:";
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
            case 'a':
                alpha = atof(optarg);
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Total epochs:        \t" << epochs << endl;
    cout << "Batch size:          \t" << batch_size << endl;
    cout << "Learning rate:       \t" << lr << endl;
    cout << "Alpha:               \t" << alpha << endl;
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

    // DataLoaderOptions have bugs.
    data::DataLoaderOptions dataloader_options;
    dataloader_options.batch_size(batch_size);
    dataloader_options.workers(workers);

    auto train_loader = data::make_data_loader(
        CompatibilityDataset(CompatibilityDataset::Mode::kTrain).
            map(data::transforms::Stack<>()),
        // batch_size
        dataloader_options
    );

    auto test_loader = data::make_data_loader(
        CompatibilityDataset(CompatibilityDataset::Mode::kTest).
            map(data::transforms::Stack<>()),
        // batch_size
        dataloader_options
    );

    optim::RMSprop optimizer(
        comp_net.parameters(),
        optim::RMSpropOptions(lr).alpha(alpha)
    );

    if (access(saved_model_folder.c_str(), 0) == -1) {
        mkdir(saved_model_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    for (int i = 0; i < symbols_n; i++) {
        symbols_w[i] = 1;
    }
    symbols_w[symbols_n] = 1.0 / symbols_n / (symbols_n - 1);
    symbols_w = symbols_w.to(device);

    for (int epoch = 1; epoch <= epochs; epoch++) {
        train(epoch, comp_net, *train_loader, optimizer, device);
        test(comp_net, *test_loader, device);
        cout << endl;
    }

    return 0;
    
}

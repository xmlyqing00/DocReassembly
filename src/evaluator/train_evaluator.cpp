#include <train_evaluator.h>

template<typename DataLoader, typename Optimizer>
void train( int epoch, 
            CompatibilityNet & cp_net, 
            DataLoader & data_loader,
            Optimizer & optimizer,
            const Device & device) {
    
    cp_net.train();
    int batch_idx = 0;

    for (auto & batch: data_loader) {
        
        Tensor data = batch.data.to(device);
        Tensor target = batch.target.to(device);
        target = squeeze(target, /*dim*/1);

        Tensor output = cp_net.forward(data);
        
        Tensor loss = nll_loss(output, target, symbols_w);
        optimizer.zero_grad();
        loss.backward();
        optimizer.step();

        if (batch_idx++ % 10 == 0) {
            printf("Train [epoch: %3d | %6d]\tLoss: %.6f\n",
                epoch, int(batch_idx * batch.data.size(0)),
                loss.item<float>());
        }

    }

    string model_path = saved_model_folder + to_string(epoch) + ".pt";
    serialize::OutputArchive output_archive;
    cp_net.save(output_archive);
    output_archive.save_to(model_path);
    cout << "Saved model at " << model_path << endl;

    if (epoch % 10 == 0) {
        model_path = saved_model_folder + "best.pt";
        output_archive.save_to(model_path);
        cout << "Saved model at " << model_path << endl;
    }

}

template<typename DataLoader>
void test(  CompatibilityNet & cp_net, 
            DataLoader & data_loader,
            const Device & device) {
    
    cp_net.eval();
    NoGradGuard no_grad;

    float test_loss = 0;
    int correct_n = 0;
    int total_n = 0;

    for (const auto & batch: data_loader) {
        
        Tensor data = batch.data.to(device);
        Tensor target = batch.target.to(device);
        target = squeeze(target, /*dim*/1);

        Tensor output = cp_net.forward(data);
        
        test_loss += nll_loss(output, target, symbols_w, Reduction::Sum).item<float>();
        auto pred = output.argmax(1);
        correct_n += pred.eq(target).sum().item<int64_t>();
        total_n += batch.data.size(0);

    }

    test_loss /= total_n;
    float avg_correct = (float)correct_n / total_n;

    printf("Test\tAvg Correct: %.4f\tAvg Loss: %.6f\n", avg_correct, test_loss);

}

int main(int argc, char ** argv) {

    // Default parameters
    int epochs = 500;
    int batch_size = 128;
    double lr = 1e-2;
    double momentum = 0.9;
    int resume = 0;

    // Parse command line parameters
    const string opt_str = "e:b:l:m:r:";
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
            case 'r':
                resume = max(0, atoi(optarg));
                break;
        }
        
        opt = getopt(argc, argv, opt_str.c_str());
    }

    cout << "Total epochs:        \t" << epochs << endl;
    cout << "Batch size:          \t" << batch_size << endl;
    cout << "Learning rate:       \t" << lr << endl;
    cout << "Momentum:            \t" << momentum << endl;
    cout << "Resume training:     \t" << resume << endl;
    cout << endl;

    CompatibilityNet cp_net;

    if (resume > 0) {
        const string model_path = saved_model_folder + to_string(resume) + ".pt";
        if (access(model_path.c_str(), 0) == -1) {
            cerr << "Model file: " << model_path << " does not exist!" << endl;
            exit(-1);
        }
        serialize::InputArchive input_archive;
        input_archive.load_from(model_path);

        cp_net.load(input_archive);
    }

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
    cp_net.to(device);

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

    // optim::RMSprop optimizer(
    //     cp_net.parameters(),
    //     optim::RMSpropOptions(lr).alpha(alpha)
    // );
    optim::SGD optimizer(
        cp_net.parameters(),
        optim::SGDOptions(lr).momentum(momentum)
    );

    if (access(saved_model_folder.c_str(), 0) == -1) {
        mkdir(saved_model_folder.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
    }

    for (int i = 0; i < symbols_n; i++) {
        symbols_w[i] = 1;
    }
    symbols_w[symbols_n] = 1.0 / symbols_n;
    // symbols_w[symbols_n] = 1;
    symbols_w = symbols_w.to(device);

    for (int epoch = resume + 1; epoch <= epochs; epoch++) {
        print_timestamp();
        train(epoch, cp_net, *train_loader, optimizer, device);
        test(cp_net, *test_loader, device);
    }

    return 0;
    
}

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

        Tensor output = comp_net.forward(data);
        cout << output.type() << " " << target.type() << endl;
        cout << output.sizes() << " " << target.sizes() << endl;

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
        // data::datasets::MNIST(dataset_path),
        batch_size
    );

    auto test_loader = data::make_data_loader(
        CompatibilityDataset(dataset_path).map(torch::data::transforms::Stack<>()), 
        // data::datasets::MNIST(dataset_path),
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


//////////////////////////////
//////////////////////////////


// #include <torch/torch.h>

// #include <cstddef>
// #include <iostream>
// #include <string>
// #include <vector>

// struct Net : torch::nn::Module {
//   Net()
//       : conv1(torch::nn::Conv2dOptions(1, 10, /*kernel_size=*/5)),
//         conv2(torch::nn::Conv2dOptions(10, 20, /*kernel_size=*/5)),
//         fc1(320, 50),
//         fc2(50, 10) {
//     register_module("conv1", conv1);
//     register_module("conv2", conv2);
//     register_module("conv2_drop", conv2_drop);
//     register_module("fc1", fc1);
//     register_module("fc2", fc2);
//   }

//   torch::Tensor forward(torch::Tensor x) {
//     x = torch::relu(torch::max_pool2d(conv1->forward(x), 2));
//     x = torch::relu(
//         torch::max_pool2d(conv2_drop->forward(conv2->forward(x)), 2));
//     x = x.view({-1, 320});
//     x = torch::relu(fc1->forward(x));
//     x = torch::dropout(x, /*p=*/0.5, /*training=*/is_training());
//     x = fc2->forward(x);
//     return torch::log_softmax(x, /*dim=*/1);
//   }

//   torch::nn::Conv2d conv1;
//   torch::nn::Conv2d conv2;
//   torch::nn::FeatureDropout conv2_drop;
//   torch::nn::Linear fc1;
//   torch::nn::Linear fc2;
// };

// struct Options {
//   std::string data_root{"/Ship01/Dataset/MNIST/"};
//   int32_t batch_size{64};
//   int32_t epochs{10};
//   double lr{0.01};
//   double momentum{0.5};
//   bool no_cuda{false};
//   int32_t seed{1};
//   int32_t test_batch_size{1000};
//   int32_t log_interval{10};
// };

// template <typename DataLoader>
// void train(
//     int32_t epoch,
//     const Options& options,
//     Net& model,
//     torch::Device device,
//     DataLoader& data_loader,
//     torch::optim::SGD& optimizer,
//     size_t dataset_size=100000) {
//   model.train();
//   size_t batch_idx = 0;
//   for (auto& batch : data_loader) {
//     auto data = batch.data.to(device), targets = batch.target.to(device);
//     optimizer.zero_grad();
//     auto output = model.forward(data);
//     auto loss = torch::nll_loss(output, targets);
//     loss.backward();
//     optimizer.step();

//     if (batch_idx++ % options.log_interval == 0) {
//       std::cout << "Train Epoch: " << epoch << " ["
//                 << batch_idx * batch.data.size(0) << "/" << dataset_size
//                 << "]\tLoss: " << loss.template item<float>() << std::endl;
//     }
//   }
// }

// template <typename DataLoader>
// void test(
//     Net& model,
//     torch::Device device,
//     DataLoader& data_loader,
//     size_t dataset_size=100000) {
//   torch::NoGradGuard no_grad;
//   model.eval();
//   double test_loss = 0;
//   int32_t correct = 0;
//   for (const auto& batch : data_loader) {
//     auto data = batch.data.to(device), targets = batch.target.to(device);
//     auto output = model.forward(data);
//     test_loss += torch::nll_loss(
//                      output,
//                      targets,
//                      /*weight=*/{},
//                      Reduction::Sum)
//                      .template item<float>();
//     auto pred = output.argmax(1);
//     correct += pred.eq(targets).sum().template item<int64_t>();
//   }

//   test_loss /= dataset_size;
//   std::cout << "Test set: Average loss: " << test_loss
//             << ", Accuracy: " << correct << "/" << dataset_size << std::endl;
// }

// struct Normalize : public torch::data::transforms::TensorTransform<> {
//   Normalize(float mean, float stddev)
//       : mean_(torch::tensor(mean)), stddev_(torch::tensor(stddev)) {}
//   torch::Tensor operator()(torch::Tensor input) {
//     return input.sub_(mean_).div_(stddev_);
//   }
//   torch::Tensor mean_, stddev_;
// };

// auto main(int argc, const char* argv[]) -> int {
//   torch::manual_seed(0);
//         const string dataset_path = "data/";
//     int batch_size = 64;
//   Options options;
//   torch::DeviceType device_type;
//   if (torch::cuda::is_available() && !options.no_cuda) {
//     std::cout << "CUDA available! Training on GPU" << std::endl;
//     device_type = torch::kCUDA;
//   } else {
//     std::cout << "Training on CPU" << std::endl;
//     device_type = torch::kCPU;
//   }
//   torch::Device device(device_type);

//   Net model;
//   model.to(device);

// //   auto train_dataset =
// //       torch::data::datasets::MNIST(
// //           options.data_root, torch::data::datasets::MNIST::Mode::kTrain)
// //           .map(Normalize(0.1307, 0.3081))
// //           .map(torch::data::transforms::Stack<>());
// //   const auto dataset_size = train_dataset.size();

//   auto train_loader = torch::data::make_data_loader(
//       torch::data::datasets::MNIST(
//           options.data_root, torch::data::datasets::MNIST::Mode::kTrain),
//         //   .map(Normalize(0.1307, 0.3081))
//         //   .map(torch::data::transforms::Stack<>()), 
//     options.batch_size);

//   auto test_loader = torch::data::make_data_loader(
//       torch::data::datasets::MNIST(
//           options.data_root, torch::data::datasets::MNIST::Mode::kTest),
//         //   .map(Normalize(0.1307, 0.3081))
//         //   .map(torch::data::transforms::Stack<>()),
//       options.batch_size);

//     //     auto train_loader = data::make_data_loader(
//     //     CompatibilityDataset(dataset_path), 
//     //     // data::datasets::MNIST(dataset_path),
//     //     batch_size
//     // );

//     // auto test_loader = data::make_data_loader(
//     //     CompatibilityDataset(dataset_path), 
//     //     batch_size
//     // );

//   torch::optim::SGD optimizer(
//       model.parameters(),
//       torch::optim::SGDOptions(options.lr).momentum(options.momentum));

//   for (size_t epoch = 1; epoch <= options.epochs; ++epoch) {
//     train(epoch, options, model, device, *train_loader, optimizer);
//     test(model, device, *test_loader);
//   }
// }
#include <compatibility_net.h>

CompatibilityNet::CompatibilityNet() :
    conv1(torch::nn::Conv2dOptions(1, 10, /*kernel_size=*/3)) {
    register_module("conv1", conv1);
}

torch::Tensor CompatibilityNet::forward(torch::Tensor & x)  {
    x = torch::relu(torch::max_pool2d(conv1->forward(x), 2));
    return x;
}
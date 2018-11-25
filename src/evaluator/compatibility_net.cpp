#include <compatibility_net.h>

CompatibilityNet::CompatibilityNet() :
    conv1(nn::Conv2dOptions(1, 10, 3)) {
    register_module("conv1", conv1);
}

Tensor CompatibilityNet::forward(Tensor & x)  {
    x = relu(max_pool2d(conv1->forward(x), 2));
    return x;
}
#ifndef COMPATIBILITY_NET_H
#define COMPATIBILITY_NET_H

#include <torch/torch.h>

class CompatibilityNet: torch::nn::Module {

public:
    torch::nn::Conv2d conv1;

    CompatibilityNet();
    torch::Tensor forward(torch::Tensor & x);
};

#endif
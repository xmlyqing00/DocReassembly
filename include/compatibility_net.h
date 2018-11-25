#ifndef COMPATIBILITY_NET_H
#define COMPATIBILITY_NET_H

#include <torch/torch.h>

using namespace torch;

class CompatibilityNet: nn::Module {

public:
    nn::Conv2d conv1;

    CompatibilityNet();
    Tensor forward(Tensor & x);
};

#endif
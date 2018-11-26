#ifndef COMPATIBILITY_NET_H
#define COMPATIBILITY_NET_H

#include <iostream>
#include <torch/torch.h>

using namespace std;
using namespace torch;

class CompatibilityNet: public nn::Module {

public:
    nn::Conv2d conv1; 
    nn::Conv2d conv2; 
    nn::Conv2d conv3;  
    nn::Conv2d conv4;  
    nn::Linear fc1;
    nn::Linear fc2;

    CompatibilityNet();
    Tensor forward(Tensor & x);
};

#endif
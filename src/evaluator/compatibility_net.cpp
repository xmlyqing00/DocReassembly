#include <compatibility_net.h>

CompatibilityNet::CompatibilityNet() : 
    conv1(nn::Conv2dOptions(  3,  32, 3)),
    conv2(nn::Conv2dOptions( 32,  64, 3)),
    conv3(nn::Conv2dOptions( 64, 128, 3)),
    conv4(nn::Conv2dOptions(128, 256, 3)),
    fc1(256, 128),
    fc2(128, 64) {
    
    // 64 -> 32
    conv1->options.padding(1);
    conv1->options.stride(2);
    register_module("conv1", conv1);

    // 32 -> 16
    conv2->options.padding(1);
    conv2->options.stride(2);
    register_module("conv2", conv2);

    // 16 -> 8
    conv3->options.padding(1);
    conv3->options.stride(2);
    register_module("conv3", conv3);

    // 8 -> 4
    conv4->options.padding(1);
    conv4->options.stride(2);
    register_module("conv4", conv4);

    // 256 -> 128
    register_module("fc1", fc1);

    // 128 -> 64
    register_module("fc2", fc2);

}

Tensor CompatibilityNet::forward(Tensor x)  {

    // cout << "input:" << endl;
    // cout << x[0][0][0] << endl;

    x = relu(conv1->forward(x));
    x = relu(conv2->forward(x));
    x = relu(conv3->forward(x));
    x = relu(conv4->forward(x));

    x = avg_pool2d(x, 4);
    x = x.view({-1, 256});
    x = relu(fc1->forward(x));
    x = fc2->forward(x);
    x = log_softmax(x, 1);

    return x;

}
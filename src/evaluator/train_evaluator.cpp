#include <train_evaluator.h>

int main() {
    torch::Tensor tensor = torch::rand({1, 1, 10, 10});
    cout << tensor << endl;

    CompatibilityNet comp_net;
    torch::Tensor y = comp_net.forward(tensor);
    cout << y << endl;
    
}
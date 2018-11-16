#include <compatibility_net.h>

int main() {
    torch::Tensor tensor = torch::rand({2, 3});
    cout << tensor << std::endl;
}
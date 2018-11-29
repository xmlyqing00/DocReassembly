#ifndef TRAIN_EVALUATOR_H
#define TRAIN_EVALUATOR_H

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <torch/torch.h>

#include <utils.h>
#include <compatibility_net.h>
#include <compatibility_dataset.h>

using namespace std;

const int symbols_n = 63;
const string saved_model_folder = "data/saved_models/";
Tensor symbols_w = torch::empty({symbols_n + 1}, kFloat32);

#endif
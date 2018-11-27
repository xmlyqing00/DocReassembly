#ifndef TRAIN_EVALUATOR_H
#define TRAIN_EVALUATOR_H

#include <iostream>
#include <torch/torch.h>

#include <compatibility_net.h>
#include <compatibility_dataset.h>

using namespace std;

const int symbols_n = 63;
const string dataset_path = "data/";
const int batch_size = 64;

#endif
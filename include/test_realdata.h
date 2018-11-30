#ifndef TEST_REALDATA_H
#define TEST_REALDATA_H

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <torch/torch.h>

#include <compatibility_net.h>

using namespace std;

const string saved_model_folder = "data/saved_models/";
const string realdata_folder = "data/cp_dataset_realdata/";
cv::Size cp_net_imgsize(64, 64);

#endif
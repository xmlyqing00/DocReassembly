#ifndef BUILD_DATASET_H
#define BUILD_DATASET_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <opencv2/opencv.hpp>

using namespace std;

enum class BuildType {
    EXTRACT_SYMBOLS,
    TRAINING,
    ALL,
};

const int noise_n = 3;
const float partition_rate = 0.9;

const string data_root = "data/";
const string symbol_folder = data_root + "symbols/";
const string dataset_folder = data_root + "cp_dataset/";


#endif
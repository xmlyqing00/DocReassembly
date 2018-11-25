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
    ALL,
    EXTRACT_SYMBOLS,
    TRAINING,
};

const int noise_n = 3;
const string symbol_folder = "data/symbols/";
const string training_folder = "data/training/";

#endif
#ifndef DEBUG_TOOL_H
#define DEBUG_TOOL_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <vector>
#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include <utils.h>

using namespace std;

enum class DebugType {
    Pixel,
    OCR_char
};

const string output_folder = "tmp/";

#endif 
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

bool show_counter_example_pixel_metric( const cv::Mat & piece_root_img,
                                        const cv::Mat & piece_best_img,
                                        const cv::Mat & piece_test_img,
                                        const double & m_score_best,
                                        cv::Mat & canvas);

bool show_counter_example_ocr_char_metric(  const cv::Mat & root_img,
                                            const cv::Mat & test_img,
                                            tesseract::TessBaseAPI * ocr,
                                            vector<cv::Mat> & out_imgs);

void find_counter_example_pixel_metric( const string & puzzle_folder,
                                        const string & case_name,
                                        int vertical_n);

void find_counter_example_ocr_char_metric(  const string & puzzle_folder,
                                            const string & case_name,
                                            int vertical_n);


const string output_folder = "tmp/";

#endif 
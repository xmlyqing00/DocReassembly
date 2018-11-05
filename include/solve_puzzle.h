#ifndef SOLVE_PUZZLE
#define SOLVE_PUZZLE

#include <unistd.h>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>

#include <stripes_solver.h>
#include <squares_solver.h>

using namespace std;

enum PuzzleType {
    STRIPES,
    SQUARES
};

#endif
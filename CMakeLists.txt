cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(DOC_REASSEMBLY)

# Find includes and libraries
include_directories(${DOC_REASSEMBLY_SOURCE_DIR}/include)
find_package(Torch REQUIRED)
find_package(OpenCV REQUIRED)
if (APPLE)
    # Add tesseract lib directory
    link_directories(/usr/local/lib)
endif()

# Set default variables
set(SRC_ROOT_PATH ${DOC_REASSEMBLY_SOURCE_DIR}/src)
set(EXECUTABLE_OUTPUT_PATH ${DOC_REASSEMBLY_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")
set(CMAKE_BUILD_TYPE Release)

# Debug cmake
# set(CMAKE_VERBOSE_MAKEFILE ON)

# utils
add_library(utils ${SRC_ROOT_PATH}/utils.cpp)
target_link_libraries(utils ${OpenCV_LIBRARIES})

# evaluator
set(EVALUATOR_PATH ${SRC_ROOT_PATH}/evaluator)
add_library(compatibility_net ${EVALUATOR_PATH}/compatibility_net.cpp)
add_executable(train-evaluator ${EVALUATOR_PATH}/train_evaluator.cpp)
target_link_libraries(train-evaluator ${TORCH_LIBRARIES})

# add-noise
add_executable(add-noise ${SRC_ROOT_PATH}/add_noise.cpp)
target_link_libraries(add-noise ${OpenCV_LIBRARIES})

# debug-tool
add_executable(debug-tool ${SRC_ROOT_PATH}/debug_tool.cpp)
target_link_libraries(debug-tool ${OpenCV_LIBRARIES} tesseract utils)

# generator
set(GENERATOR_PATH ${SRC_ROOT_PATH}/generator)
aux_source_directory(${GENERATOR_PATH} GENERATOR_SRC)
add_executable(generator ${GENERATOR_SRC})
target_link_libraries(generator ${OpenCV_LIBRARIES} tesseract utils)

# solver
set(SOLVER_PATH ${SRC_ROOT_PATH}/solver)
aux_source_directory(${SOLVER_PATH} SOLVER_SRC)
add_executable(solver ${SOLVER_SRC})
target_link_libraries(solver ${OpenCV_LIBRARIES} tesseract utils)
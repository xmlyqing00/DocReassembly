cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(DOC_REASSEMBLY)

# Set default variables
set(SRC_ROOT_PATH ${DOC_REASSEMBLY_SOURCE_DIR}/src)
set(CMAKE_CXX_STANDARD 11)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(EXECUTABLE_OUTPUT_PATH ${DOC_REASSEMBLY_SOURCE_DIR}/bin/release)
else()
    set(DEBUG_FLAG "-DDEBUG")
    set(EXECUTABLE_OUTPUT_PATH ${DOC_REASSEMBLY_SOURCE_DIR}/bin/debug)
endif()

# Find includes and libraries
include_directories(${DOC_REASSEMBLY_SOURCE_DIR}/include)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if (APPLE)
    # Add tesseract lib directory
    link_directories(/usr/local/lib)
endif()

# OpenMP
if(APPLE)
    set(CMAKE_C_COMPILER "/usr/local/Cellar/llvm/8.0.0_1/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/Cellar/llvm/8.0.0_1/bin/clang++")
    set(OPENMP_LIBRARIES "/usr/local/Cellar/llvm/8.0.0_1/lib")
    set(OPENMP_INCLUDES "/usr/local/Cellar/llvm/8.0.0_1/include")

    set(OpenMP_C "${CMAKE_C_COMPILER}")
    set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
    set(OpenMP_C_LIB_NAMES "libomp" "libgomp")
    set(OpenMP_libomp_LIBRARY ${OpenMP_C_LIB_NAMES})
    set(OpenMP_libgomp_LIBRARY ${OpenMP_C_LIB_NAMES})
    
    set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
    set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
    set(OpenMP_CXX_LIB_NAMES "libomp" "libgomp")
    set(OpenMP_libomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
    set(OpenMP_libgomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
endif()

find_package(OpenMP REQUIRED)
include_directories(${OpenMP_INCLUDE_DIRS})
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Debug cmake
set(CMAKE_VERBOSE_MAKEFILE ON)

# utils
add_library(utils ${SRC_ROOT_PATH}/utils.cpp)
target_compile_definitions(utils PUBLIC ${DEBUG_FLAG})
target_link_libraries(utils ${OpenCV_LIBRARIES})

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

add_library(path_manager ${SOLVER_PATH}/path_manager.cpp)
add_library(KM ${SOLVER_PATH}/KM.cpp)

add_executable(solver ${SOLVER_SRC})
target_compile_definitions(solver PUBLIC ${DEBUG_FLAG})
target_link_libraries(solver ${OpenCV_LIBRARIES} tesseract utils path_manager KM)

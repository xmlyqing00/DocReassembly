#!/bin/bash

echo [cmd] mkdir build
mkdir build
cd build

echo [cmd] cmake -DCMAKE_PREFIX_PATH=/Users/yq/Sources/libtorch ..
cmake -DCMAKE_PREFIX_PATH=/Users/yq/Sources/libtorch ..

echo [cmd] make -j12
make -j12
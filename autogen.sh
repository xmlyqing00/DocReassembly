#!/bin/bash

echo [cmd] mkdir build
mkdir build
cd build

echo [cmd] cmake -DLibtorch_REL_PATH=../libtorch/ ..
cmake -DLibtorch_REL_PATH=../libtorch/ ..

echo [cmd] make -j12
make -j12
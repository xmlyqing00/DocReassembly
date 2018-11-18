#!/bin/bash

if [ "$1" == "clean" ]; then
    echo [task] Clean project.
    cd build
    echo [cmd ] make clean
    make clean
    cd ..
    echo [cmd ] rm -rf build
    rm -rf build
    echo [task] Clean Project done.
else
    echo [task] Compile project.
    echo [ cmd] mkdir build
    mkdir build
    cd build

    echo [ cmd] cmake -DLibtorch_REL_PATH=../libtorch/ ..
    cmake -DLibtorch_REL_PATH=../libtorch/ ..

    echo [ cmd] make -j12
    make -j12
    echo [task] Compile project done.
fi

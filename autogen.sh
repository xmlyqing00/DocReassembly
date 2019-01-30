#!/bin/bash

if [ $# == 0 ]; then
    echo "Enter option either release or debug."
    echo "End."
fi  

if [ "$1" == "release" ]; then
    echo "[task] Compile project in RELEASE mode".
    echo "[ cmd] mkdir build/release"
    mkdir -p build/release
    cd build/release

    echo "[ cmd] cmake -DCMAKE_BUILD_TYPE=Release ../../"
    # cmake -DLibtorch_REL_PATH=../libtorch/ -DCMAKE_BUILD_TYPE=Release ../../
    cmake -DCMAKE_BUILD_TYPE=Release ../../

    echo "[ cmd] make -j12"
    make -j12
    echo "[task] Compile project done."
fi

if [ "$1" == "debug" ]; then
    echo "[task] Compile project in DEBUG mode".
    echo "[ cmd] mkdir build/debug"
    mkdir -p build/debug
    cd build/debug

    echo "[ cmd] -DCMAKE_BUILD_TYPE=Debug ../../"
    # cmake -DLibtorch_REL_PATH=../libtorch/ -DCMAKE_BUILD_TYPE=Debug ../..
    cmake -DCMAKE_BUILD_TYPE=Debug ../..

    echo "[ cmd] make -j12"
    make -j12
    echo "[task] Compile project done."
fi

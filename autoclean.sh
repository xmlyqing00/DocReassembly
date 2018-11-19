#!/bin/bash

if [ $# == 0 ]; then
    echo "Enter option either release or debug."
    echo "End."
fi  

if [ "$1" == "release" ]; then
    
    echo "[task] Clean project RELEASE files."
    
    if [ -d "build/release" ]; then
        cd build/release
        echo "[cmd ] build/release make clean"
        make clean
        cd ../../
    fi
    
    echo "[cmd ] rm -rf build/release bin/release"
    rm -rf build/release
    rm -rf bin/release
    echo "[task] Clean Project RELEASE files done."

fi

if [ "$1" == "debug" ]; then
    
    echo "[task] Clean project DEBUG files."
    
    if [ -d "build/debug" ]; then
        cd build/debug
        echo "[cmd ] build/debug make clean"
        make clean
        cd ../../
    fi
    
    echo "[cmd ] rm -rf build/debug bin/debug"
    rm -rf build/debug
    rm -rf bin/debug
    echo "[task] Clean Project DEBUG files done."

fi
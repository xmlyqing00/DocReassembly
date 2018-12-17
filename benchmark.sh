#!/bin/bash

test_n=4
nums=(20 30 40 50)
samples=(30 50 70 100)
metrics=(0 2)
compositions=(0 1)

if [ $# == 0 ]; then
    echo "Enter a test case name."
    echo "End."
    exit
fi  

echo "Test case: "$var
for i in $(seq 0 ${test_n}); do
    ./bin/release/generator -t $1 -n ${nums[i]}
    echo " "

    for metric in ${metrics[@]}; do
        for comp in ${compositions[@]}; do
            ./bin/release/solver \
                -t $1 \
                -n ${nums[i]} \
                -m ${metric} \
                -c ${comp} \
                -s ${samples[i]}
            echo "  "
        done
    done
done
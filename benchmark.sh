#!/bin/bash

generator_n=3
generator_nums=(20 30 40 60)

test_n=4
nums=(20 30 40 60)
samples=(100 300 1000 8000)
metrics=(0 2)
compositions=(2)

if [ $# == 0 ]; then
    echo "Enter a test case name."
    echo "End."
    exit
fi  

if [ "$1" == "GA" ]; then
    echo "Add seams on GA solutions."

    # test=("doc0" "doc3" "doc7" "doc10" "doc11" "doc12" "doc13" "doc14" "doc15" "doc16" "doc17" "doc18" "doc19" "doc20" "doc21" "doc23" "doc24")
    test=("doc7")
    for var in ${test[@]}; do
        for i in $(seq 0 ${test_n}); do
            ./bin/release/debug-tool -d 2 -o ${var}_${nums[i]}
        done
    done

    exit
fi

echo "Test case: "$1
# for i in $(seq 0 ${generator_n}); do
    # ./bin/release/generator -t $1 -n ${generator_nums[i]}
    # echo " "
# done

for metric in ${metrics[@]}; do
    for comp in ${compositions[@]}; do
        for i in $(seq 0 ${test_n}); do
            ./bin/release/solver \
                -t $1 \
                -n ${nums[i]} \
                -m ${metric} \
                -c ${comp} \
                -s ${samples[i]} \
                -b
            echo "  "
        done
    done
done
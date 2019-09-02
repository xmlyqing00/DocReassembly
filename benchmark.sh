#!/bin/bash

# test_n=3
# nums=(20 30 40 60)
# samples=(150 300 1000 8000)

test_n=0
nums=(20)
samples=(150)

# method_n=1
# metrics=(0 1)
# comps=(0 0)

method_n=0
metrics=(2)
comps=(2)

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

if [ "$1" == "doc" ]; then
    echo "Run doc dataset."
    test_files=("doc0" "doc3" "doc7" "doc10" "doc12" "doc13" "doc14" "doc15" "doc16" "doc17" "doc18" "doc20" "doc21" "doc23" "doc24")    

fi

if [ "$1" == "noise" ]; then
    echo "Run noise dataset."
    test_files=("doc16_noise20" "doc18_noise20")
fi

if [ "$1" == "doc" ] || [ "$1" == "noise" ]; then
    for test_name in ${test_files[@]}; do

        echo "Test case:" ${test_name} "==========="

        if [ "$2" == "gen" ]; then
            echo "Generate puzzles"
            for i in $(seq 0 ${test_n}); do
                ./bin/release/generator -t ${test_name} -n ${nums[i]}
                echo " "
            done
        fi

        for i in $(seq 0 ${method_n}); do
            for j in $(seq 0 ${test_n}); do
                echo "Test args:" ${test_name} ${nums[j]} ${metrics[i]} ${comps[i]} ${samples[j]}
                ./bin/release/solver \
                    -t ${test_name} \
                    -n ${nums[j]} \
                    -m ${metrics[i]} \
                    -c ${comps[i]} \
                    -s ${samples[j]} \
                    -b
                echo "  "
            done
        done
    done

    exit

fi



echo "Test case: "$1
for i in $(seq 0 ${test_n}); do
    ./bin/release/generator -t $1 -n ${nums[i]}
    echo " "
done

for i in $(seq 0 ${method_n}); do
    for j in $(seq 0 ${test_n}); do
        echo "Test args:" ${test_name} ${nums[j]} ${metrics[i]} ${comps[i]} ${samples[j]}
        ./bin/release/solver \
            -t $1 \
            -n ${nums[j]} \
            -m ${metrics[i]} \
            -c ${comps[i]} \
            -s ${samples[j]} \
            -b
        echo "  "
    done
done
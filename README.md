# StripeReassembly

This repository is a C++ implementation for 
```
    Reassembling Shredded Document Stripes Using Word-path Metric and Greedy Composition Optimal Matching Solver
```
If you use these codes in your research, please cite the paper.

## Environment

We build and evaluate our codes under Ubuntu 18.04 and Mac OS X 10.14.5. The following packages are used in this repository:
1. OpenCV: 3.2.0
2. OpenMP: 2.1.1
3. CMake: 3.5
4. g++: 7.4.0
5. Python: 3.6.8

## DocDataset description

Click [here](http://t.lyq.me?d=DocDataset) to download the `DocDataset`. Unzip the package and copy the `gt` and `stripes` into the `/data/` folder of the repository.

`DocDataset` contains:
1. 60 striped document puzzles with four types of complexities of 20, 30, 40, and 60 stripes. They are named as `doc*_*`.
2. 3 physically shredded document puzzles. They are named as `real*_*`.
3. 1 randomly oriented puzzle named `doc3_36`.

The comparison performance results are reported in the paper.

## Usage

### Compile

`CMakeLists.txt` 
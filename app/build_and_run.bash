#!/bin/bash
clear

printf "\e[1;36m----- building and installing app -----\e[0m\n"
rm -rf build exe
mkdir build
cd build
cmake ..
cmake --build . --parallel 16
cmake --install .

printf "\e[1;36m----- running app -----\e[0m\n"
cd ../exe
./run_get_weight_matrices

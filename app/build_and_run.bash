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
#./run_get_weight_matrices
./run_get_projection_maps front 0.7 0.8 -150 -100
#./run_get_projection_maps left 0.4 0.8 150 0
#./run_get_projection_maps back 0.8 1 0 100
#./run_get_projection_maps right 0.4 1 0 0
#./run_calibrate_camera
#./test_cameras
#./template_calibrate_camera

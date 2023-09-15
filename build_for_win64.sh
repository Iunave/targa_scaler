#!/bin/bash

[ -e "build_windows" ] || mkdir build_windows
x86_64-w64-mingw32-g++ -std=c++20 -O3 -march=native -I source -I third_party source/downscale.cpp source/system.cpp source/algorithm.cpp -o build_windows/downscale
strip -s build_windows/downscale.exe

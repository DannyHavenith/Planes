#!/bin/bash

# This script needs the variable EMSDK to be set to the path of the emsdk directory.
mkdir -p emscripten_build && cd emscripten_build
cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Debug -G "Ninja" ..

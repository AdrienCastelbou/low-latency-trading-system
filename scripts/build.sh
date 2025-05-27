#!/bin/bash

BUILD_TYPE=${1:-Debug}

echo "Building in $BUILD_TYPE mode..."

cmake . -B build
cmake . -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build build

echo "Build complete!"

#!/bin/bash

cd swpp202101-interpreter
mkdir -p build
cd build
cmake ../
make -j
cp sf-interpreter ../../bin
cd ..
rm -rf build

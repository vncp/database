#!/usr/bin/env bash

# init and recurse submodules
git submodule update --init --recursive

# generate config
cd protobuf
./autogen.sh

# build and install C++ Protobuf runtime and compiler
./configure --prefix=$(pwd)
make -j$(nproc)
make check
make install
sudo ldconfig


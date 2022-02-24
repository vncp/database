#!/usr/bin/env bash

# get C++ tar
PROJECT_ROOT=$(pwd)
mkdir ./etc && cd ./etc
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

# extract 
tar -xzvf protobuf-cpp-3.19.4.tar.gz
mv protobuf-3.19.4/ protobuf/

# build and install C++ Protobuf runtime and compiler
cd ./protobuf
./configure --prefix=$PROJECT_ROOT/etc
make -j$(nproc)
make check
make install

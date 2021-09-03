#!/bin/bash

set -e

mkdir -p cmake/build 2> /dev/null || echo "build folder exists, skipping"
cd cmake/build

mkdir _generated  2> /dev/null || echo "generated folder exists, skipping"

protoc -I=../../protos --cpp_out=_generated --grpc_out=_generated --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ../../protos/ping.proto

cd ../..

echo "ALL GOOD"

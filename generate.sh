#!/bin/bash

set -e

mkdir cmake 2> /dev/null || echo "build folder exists, skipping"

pushd cmake

mkdir _generated  2> /dev/null || echo "generated folder exists, skipping"

protoc -I=../protos --cpp_out=_generated --grpc_out=_generated --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ../protos/example.proto

popd

echo "ALL GOOD"

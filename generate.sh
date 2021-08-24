#!/bin/bash

# protoc -I=./protos --cpp_out=./cpp_out --grpc_out=grpc_out --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/helloworld.proto

protoc -I=./protos --cpp_out=generated --grpc_out=generated --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./protos/helloworld.proto


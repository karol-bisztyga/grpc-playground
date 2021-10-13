PROJECT(CppApp C CXX)

cmake_minimum_required(VERSION 3.21)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)

#include(./common.cmake)

# FIND LIBS
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(hw_proto "./protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

file(GLOB GENERATED_CODE "./cmake/build/_generated/*.cc")
file(GLOB SOURCE_CODE "./src/*.cpp")

include_directories(
  ./src
  ./cmake/build/_generated
)

# CLIENT
add_executable(
  client

  ${GENERATED_CODE}
  ${SOURCE_CODE}
)

target_link_libraries(
  client
  
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)

install(
  TARGETS client
  RUNTIME DESTINATION bin/
)

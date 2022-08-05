PROJECT(CppApp C CXX)

cmake_minimum_required(VERSION 3.21)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)

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

find_package(OpenSSL REQUIRED)

# Proto file
get_filename_component(hw_proto "./protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

file(GLOB GENERATED_CODE "./cmake/build/_generated/*.cc")
file(GLOB MUTUAL_SOURCE_CODE "./src/lib/*.cpp")
file(GLOB CLIENT_SOURCE_CODE "./src/client/*.cpp")
file(GLOB INNER_SERVER_SOURCE_CODE "./src/inner_server/*.cpp")
file(GLOB OUTER_SERVER_SOURCE_CODE "./src/outer_server/*.cpp")

include_directories(
  ./src
  ./src/lib
  ./src/client
  ./src/lib/client-base-reactors
  ./src/lib/server-base-reactors
  ./src/inner_server
  ./src/outer_server
  ./cmake/build/_generated
)

# CLIENT
add_executable(
  client

  ${GENERATED_CODE}
  ./src/client.cpp
  ${MUTUAL_SOURCE_CODE}
  ${SOURCE_CODE}
)

target_link_libraries(
  client
  
  ${_GRPC_GRPCPP}
  OpenSSL::SSL
)

install(
  TARGETS client
  RUNTIME DESTINATION bin/
)

# INNER SERVER
add_executable(
  inner_server

  ${GENERATED_CODE}
  ./src/inner_server.cpp
  ${MUTUAL_SOURCE_CODE}
  ${INNER_SERVER_SOURCE_CODE}
)

target_link_libraries(
  inner_server
  
  ${_GRPC_GRPCPP}
  OpenSSL::SSL
)

install(
  TARGETS inner_server
  RUNTIME DESTINATION bin/
)

# OUTER SERVER
add_executable(
  outer_server

  ${GENERATED_CODE}
  ./src/outer_server.cpp
  ${MUTUAL_SOURCE_CODE}
  ${OUTER_SERVER_SOURCE_CODE}
)

target_link_libraries(
  outer_server
  
  ${_GRPC_GRPCPP}
  OpenSSL::SSL
)

install(
  TARGETS outer_server
  RUNTIME DESTINATION bin/
)

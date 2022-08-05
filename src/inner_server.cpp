#include "InnerServiceImpl.h"

#include "Constants.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <thread>

void RunServer() {
  InnerServiceImpl innerService;

  std::string addr = LISTEN_ADDRESS + ":" + INNER_SERVER_PORT;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(
      addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&innerService);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
            << "]"
            << "server listening at :" << addr << std::endl;

  server->Wait();
}

int main(int argc, char **argv) {
  RunServer();

  return 0;
}

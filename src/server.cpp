#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class GreeterServiceImpl final : public example::ExampleService::Service
{
  grpc::Status ExchangeData(::grpc::ServerContext *context, grpc::ServerReaderWriter<example::DataResponse, example::DataRequest> *stream) override {
    example::DataRequest request;
    example::DataResponse response;

    std::vector<std::string> responses = {"", "response 4", "response 3", "response 2", "response 1"};

    while(stream->Read(&request)) {
      std::string receivedData = request.data();
      std::string dataToSend = responses.back();
      response.set_data(dataToSend);
      responses.pop_back();

      std::cout << "received data: [" << receivedData << "], sending data: [" << dataToSend << "]" << std::endl;
      stream->Write(response);
    }
    std::cout << "no more reads, terminating session" << std::endl;
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("localhost:50051");
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}

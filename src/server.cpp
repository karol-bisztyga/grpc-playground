#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include "ReactorBase.h"

class ExchangeReactor : public ReactorBase<example::DataRequest, example::DataResponse>
{
  std::vector<std::string> responses = {"", "res 4", "res 3", "res 2", "res 1"};
  size_t i=0;
public:
  grpc::Status handleRequest(example::DataRequest request, example::DataResponse *response) override
  {
    std::cout << "received: " << request.data() << "/ gonna respond: " << this->responses.back() << std::endl;
    if (this->responses.empty()) {
      return grpc::Status(grpc::StatusCode::CANCELLED, "empty response");
    }
    if (i>1)
    {
      throw std::runtime_error("test error");
    }
    response->set_data(this->responses.back() + "/" + std::to_string(i));
    this->responses.pop_back();
    ++i;
    return grpc::Status::OK;
  }
};

class ExampleServiceImpl final : public example::ExampleService::CallbackService
{
public:
  grpc::ServerBidiReactor<example::DataRequest, example::DataResponse> *
  ExchangeData(grpc::CallbackServerContext *context) override
  {
    return new ExchangeReactor();
  }
};

void RunServer()
{
  std::string server_address("localhost:50051");
  ExampleServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();

  return 0;
}

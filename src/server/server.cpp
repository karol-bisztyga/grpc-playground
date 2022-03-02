#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include "BidiReactorBase.h"
#include "ReadReactorBase.h"
#include "WriteReactorBase.h"

class ExchangeBidiReactor : public BidiReactorBase<example::DataRequest, example::DataResponse>
{
  std::vector<std::string> responses = {"res 4", "res 3", "res 2", "res 1"};
  size_t i = 0;

public:
  std::unique_ptr<grpc::Status> handleRequest(example::DataRequest request, example::DataResponse *response) override
  {
    std::cout << "received: " << request.data() << "/ gonna respond: " << this->responses.back() << std::endl;
    if (this->responses.empty())
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    if (i > 1)
    {
      throw std::runtime_error("test error");
    }
    response->set_data(this->responses.back() + "/" + std::to_string(i));
    this->responses.pop_back();
    ++i;
    return nullptr;
  }
};

class ExchangeReadReactor : public ReadReactorBase<example::DataRequest, example::DataResponse>
{
  size_t i = 0;
public:
  using ReadReactorBase<example::DataRequest, example::DataResponse>::ReadReactorBase;

  std::unique_ptr<grpc::Status> readRequest(example::DataRequest request) override
  {
    // this->response can be modified and it will be sent to the client in the end
    std::cout << "received request [" << request.data() << "]" << std::endl;
    if (i > 3)
    {
      throw std::runtime_error("error: too many data chunks");
    }
    if (request.data().empty())
    {
      std::string resp = "thank you";
      std::cout << "setting response: " << resp << std::endl;
      this->response->set_data(resp);
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    ++i;
    return nullptr;
  }
};

class ExchangeWriteReactor : public WriteReactorBase<example::DataRequest, example::DataResponse>
{
  size_t i = 0;
public:
  using WriteReactorBase<example::DataRequest, example::DataResponse>::WriteReactorBase;

  std::unique_ptr<grpc::Status> writeResponse(example::DataResponse *response) override
  {
    // this->request can be accessed as read-only
    std::cout << "preparing response with request [" << this->request.data() << "]" << std::endl;
    if (this->i > 3) {
      std::cout << "terminating stream" << std::endl;
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string responseStr = "response " + std::to_string(++this->i) + "(" + this->request.data() + ")";
    std::cout << "sending response: [" << responseStr << "]" << std::endl;
    response->set_data(responseStr);
    return nullptr;
  }
};

class ExampleServiceImpl final : public example::ExampleService::CallbackService
{
public:
  grpc::ServerBidiReactor<example::DataRequest, example::DataResponse> *
  ExchangeData(grpc::CallbackServerContext *context) override
  {
    return new ExchangeBidiReactor();
  }

  grpc::ServerReadReactor<example::DataRequest> *OneWayStreamClientToServer(
      grpc::CallbackServerContext *context, example::DataResponse *response) override
  {
    return new ExchangeReadReactor(response);
  }
  grpc::ServerWriteReactor<example::DataResponse> *OneWayStreamServerToClient(
      grpc::CallbackServerContext *context, const example::DataRequest *request) override
  {
    ExchangeWriteReactor * reactor = new ExchangeWriteReactor(request);
    reactor->NextWrite();
    return reactor;
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

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include "ServerBidiReactorBase.h"
#include "ServerReadReactorBase.h"
#include "ServerWriteReactorBase.h"

class ExchangeBidiReactor : public ServerBidiReactorBase<example::DataRequest, example::DataResponse>
{
  std::vector<std::string> responses = {"res 4", "res 3", "res 2", "res 1"};
  size_t i = 0;

public:
  std::unique_ptr<ServerBidiReactorStatus> handleRequest(example::DataRequest request, example::DataResponse *response) override
  {
    if (this->responses.empty())
    {
      return std::make_unique<ServerBidiReactorStatus>(grpc::Status::OK);
    }
    std::cout << "received: " << request.data() << "/ gonna respond: " << this->responses.back() << std::endl;
    // if (i > 1)
    // {
    //   throw std::runtime_error("test error");
    // }
    response->set_data(this->responses.back() + "/" + std::to_string(i));
    this->responses.pop_back();
    ++i;
    return nullptr;
  }

  void doneCallback() override
  {
    std::cout << "done CLB: " << this->status.status.error_code() << std::endl;
  }
};

class ExchangeReadReactor : public ServerReadReactorBase<example::DataRequest, example::DataResponse>
{
  size_t i = 0;
public:
  using ServerReadReactorBase<example::DataRequest, example::DataResponse>::ServerReadReactorBase;

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

class ExchangeWriteReactor : public ServerWriteReactorBase<example::DataRequest, example::DataResponse>
{
  size_t i = 0;
public:
  using ServerWriteReactorBase<example::DataRequest, example::DataResponse>::ServerWriteReactorBase;

  std::unique_ptr<grpc::Status> writeResponse(example::DataResponse *response) override
  {
    // this->request can be accessed as read-only
    std::cout << "preparing response with request [" << this->request.data() << "]" << std::endl;
    if (this->i > 5) {
      std::cout << "terminating stream" << std::endl;
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string responseStr = "response " + std::to_string(this->i) + "(" + this->request.data() + ")";
    std::cout << "sending response: [" << responseStr << "]" << std::endl;
    response->set_data(responseStr);
    ++this->i;
    return nullptr;
  }

  void doneCallback() override
  {
    std::cout << "done CLB" << std::endl;
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

  grpc::ServerUnaryReactor *Unary(grpc::CallbackServerContext *context, const example::DataRequest *request, example::DataResponse *response) override {
    std::cout << "unary for request [" << request->data() << "]" << std::endl;
    response->set_data("response #1 for request ["+ request->data() +"]");
    auto *reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "test error"));
    // reactor->Finish(grpc::Status::OK);
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

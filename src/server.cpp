#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

struct EndConnectionError : public std::exception
{
  const char *what() const throw()
  {
    return "connection ended";
  }
};

template <class Request, class Response>
class ReactorBase : public grpc::ServerBidiReactor<Request, Response>
{
  Request request;
  Response response;
  bool finished = false;

  void finish()
  {
    if (this->finished)
    {
      return;
    }
    this->finished = true;
    this->Finish(grpc::Status::OK);
  }

public:
  ReactorBase()
  {
    this->StartRead(&this->request);
  }

  void OnDone() override
  {
    GPR_ASSERT(this->finished);
    delete this;
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      this->finish();
      return;
    }
    try
    {
      this->response = this->handleRequest(this->request);
      this->StartWrite(&this->response);
    }
    catch (EndConnectionError &e)
    {
      this->finish();
    }
  }

  void OnWriteDone(bool ok) override
  {
    if (!ok)
    {
      gpr_log(GPR_ERROR, "Server write failed");
      return;
    }
    this->StartRead(&this->request);
  }

  virtual Response handleRequest(Request request) = 0;
};

class ExchangeReactor : public ReactorBase<example::DataRequest, example::DataResponse>
{
  size_t id = 0;

public:
  example::DataResponse handleRequest(example::DataRequest request) override
  {
    ++this->id;
    std::cout << "here CLB: " << request.data() << "/" << this->id << std::endl;
    if (this->id > 3 || request.data() == "exit")
    {
      this->id = 0;
      throw EndConnectionError();
    }
    example::DataResponse response;
    response.set_data("response " + std::to_string(this->id));
    return response;
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

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

  void finish(grpc::Status status = grpc::Status::OK)
  {
    std::cout << "=> finish" << std::endl;
    if (this->finished)
    {
      return;
    }
    this->finished = true;
    this->Finish(status);
  }

public:
  ReactorBase()
  {
    this->StartRead(&this->request);
  }

  void OnDone() override
  {
    std::cout << "=> done" << std::endl;
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
    catch (std::runtime_error &e)
    {
      std::cout << "error " << e.what() << std::endl;
      this->finish(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
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
  std::vector<std::string> responses = {"", "res 4", "res 3", "res 2", "res 1"};
  size_t i=0;
public:
  example::DataResponse handleRequest(example::DataRequest request) override
  {
    std::cout << "received: " << request.data() << "/ gonna respond: " << this->responses.back() << std::endl;
    if (this->responses.empty()) {
      throw EndConnectionError();
    }
    if (i>1)
    {
      throw std::runtime_error("test error");
    }
    example::DataResponse response;
    response.set_data(this->responses.back());
    this->responses.pop_back();
    ++i;
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

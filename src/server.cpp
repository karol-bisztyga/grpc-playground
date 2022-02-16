#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class Reactor : public grpc::ServerBidiReactor<example::DataRequest, example::DataResponse>
{
  example::DataRequest request;
  example::DataResponse response;
  bool finished = false;

  size_t responseID = 0;

  void finish()
  {
    if (this->finished) {
      return;
    }
    this->finished = true;
    Finish(grpc::Status::OK);
  }

public:
  Reactor() {
    StartRead(&this->request);
  }

  void OnDone() override
  {
    GPR_ASSERT(this->finished);
    delete this;
  }

  void OnReadDone(bool ok) override
  {
    if (!ok) {
      this->finish();
      return;
    }
    std::cout << "data from client: [" << this->request.data() << "] " << this->responseID << std::endl;

    this->response.set_data("response " + std::to_string(++this->responseID));
    StartWrite(&this->response);
  }

  void OnWriteDone(bool ok) override
  {
    if (!ok) {
      gpr_log(GPR_ERROR, "Server write failed");
      return;
    }
    if (this->responseID >= 3) {
      std::cout << "already sent 3 responses, terminating connection" << std::endl;
      this->finish();
    }
    StartRead(&this->request);
  }
};

class ExampleServiceImpl final : public example::ExampleService::CallbackService
{
  grpc::ServerBidiReactor<example::DataRequest, example::DataResponse> *
  ExchangeData(grpc::CallbackServerContext *context) override
  {
    return new Reactor();
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

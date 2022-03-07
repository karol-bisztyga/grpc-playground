#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class BidiReactor : public grpc::ClientBidiReactor<example::DataRequest, example::DataResponse>
{
  grpc::ClientContext context;
  example::DataRequest request;
  example::DataResponse response;
  grpc::Status status;
  bool done = false;
  bool initialized = 0;

public:
  BidiReactor(example::ExampleService::Stub *stub)
  {
    stub->async()->ExchangeData(&this->context, this);
  }

  void NextWrite(std::string msg)
  {
    this->request.set_data(msg);
    StartWrite(&this->request);
    if (!this->initialized)
    {
      StartCall();
      this->initialized = true;
    }
  }

  void terminate(const grpc::Status &status) {
    if (this->done) {
      return;
    }
    this->status = status;
    std::cout << "DONE [code=" << status.error_code() << "][err=" << status.error_message() << "]" << std::endl;
    this->done = true;
  }

  bool isDone() {
    return this->done;
  }

  void OnWriteDone(bool ok) override
  {
    StartRead(&this->response);
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      if (this->done) {
        return;
      }
      std::cout << "error - terminating: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
      this->terminate(grpc::Status(grpc::StatusCode::UNKNOWN, "read error"));
      return;
    }
    std::cout << "Got message [" << this->response.data() << "]" << std::endl;
  }

  void OnDone(const grpc::Status &status) override
  {
    this->terminate(status);
  }
};

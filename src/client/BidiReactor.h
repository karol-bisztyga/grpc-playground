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
    if (this->initialized && this->response.data().empty())
    {
      std::cout << "empty message - terminating" << std::endl;
      this->StartWritesDone();
      return;
    }
    this->request.set_data(msg);
    StartWrite(&this->request);
    if (!this->initialized)
    {
      StartCall();
      this->initialized = true;
    }
  }

  bool isDone() {
    return this->done;
  }

  void OnWriteDone(bool ok) override
  {
    std::cout << "Done writing: " << this->request.data() << std::endl;
    StartRead(&this->response);
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      std::cout << "error - terminating: " << "/" << this->status.error_message() << std::endl;
      this->StartWritesDone();
      return;
    }
    std::cout << "Got message [" << this->response.data() << "]" << std::endl;
  }

  void OnDone(const grpc::Status &status) override
  {
    this->status = status;
    std::cout << "DONE [code=" << status.error_code() << "][err=" << status.error_message() << "]" << std::endl;
    this->done = true;
  }
};

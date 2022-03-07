#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class WriteReactor : public grpc::ClientWriteReactor<example::DataRequest>
{
  grpc::ClientContext context;
  example::DataRequest request;
  grpc::Status status;
  bool done = false;
  bool initialized = 0;

public:
  example::DataResponse response;
  WriteReactor(example::ExampleService::Stub *stub)
  {
    stub->async()->OneWayStreamClientToServer(&this->context, &this->response, this);
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

  void terminate(const grpc::Status &status)
  {
    if (this->done) {
      return;
    }
    this->status = status;
    std::cout << "DONE [code=" << status.error_code() << "][err=" << status.error_message() << "]" << std::endl;
    this->done = true;
  }

  bool isDone()
  {
    return this->done;
  }

  void OnDone(const grpc::Status &status) override
  {
    this->terminate(status);
  }
};

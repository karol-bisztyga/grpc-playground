#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class ReadReactor : public grpc::ClientReadReactor<example::DataResponse>
{
  grpc::ClientContext context;
  example::DataResponse response;
  grpc::Status status;
  bool done = false;
  size_t counter = 0;

public:
  ReadReactor(example::ExampleService::Stub *stub, const example::DataRequest &request)
  {
    stub->async()->OneWayStreamServerToClient(&this->context, &request, this);
    this->StartRead(&this->response);
    this->StartCall();
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      std::cout << "terminating" << std::endl;
      return;
    }
    std::cout << "Got message [" << this->response.data() << "]" << std::endl;
    this->StartRead(&this->response);
  }

  void OnDone(const grpc::Status &status) override
  {
    this->status = status;
    std::cout << "DONE [code=" << status.error_code() << "][err=" << status.error_message() << "]" << std::endl;
    this->done = true;
  }

  bool isDone() {
    return this->done;
  }
};

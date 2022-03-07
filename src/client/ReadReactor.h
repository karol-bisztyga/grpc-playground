#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include <functional>

class ReadReactor : public grpc::ClientReadReactor<example::DataResponse>
{
  grpc::ClientContext context;
  example::DataResponse response;
  grpc::Status status;
  bool done = false;
  size_t counter = 0;
  std::function<bool(const example::DataResponse&)> shouldConnectionTerminate = nullptr;

  void terminate(const grpc::Status &status)
  {
    if (this->done) {
      return;
    }
    this->status = status;
    std::cout << "DONE [code=" << status.error_code() << "][err=" << status.error_message() << "]" << std::endl;
    this->done = true;
  }

public:
  ReadReactor(example::ExampleService::Stub *stub, const example::DataRequest &request, std::function<bool(const example::DataResponse &)> shouldConnectionTerminate = nullptr) : shouldConnectionTerminate(shouldConnectionTerminate)
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
      this->terminate(grpc::Status(grpc::StatusCode::UNKNOWN, "read error"));
      return;
    }
    std::cout << "Got message [" << this->response.data() << "]" << std::endl;
    if (this->shouldConnectionTerminate != nullptr && this->shouldConnectionTerminate(this->response))
    {
      this->terminate(grpc::Status::OK);
      return;
    }
    this->StartRead(&this->response);
  }

  void OnDone(const grpc::Status &status) override
  {
    this->terminate(status);
  }

  bool isDone() {
    return this->done;
  }
};

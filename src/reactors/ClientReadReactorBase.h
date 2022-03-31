#include <grpcpp/grpcpp.h>

#include <functional>

template <class Request, class Response>
class ClientReadReactorBase : public grpc::ClientReadReactor<Response>
{
  Response response;
  grpc::Status status;
  bool done = false;

  void terminate(const grpc::Status &status)
  {
    this->status = status;
    if (this->done) {
      return;
    }
    this->done = true;
  }

public:
  Request request;
  grpc::ClientContext context;

  void start() {
    this->StartRead(&this->response);
    this->StartCall();
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      this->terminate(grpc::Status(grpc::StatusCode::UNKNOWN, "read error"));
      return;
    }
    std::unique_ptr<grpc::Status> status = this->readResponse(this->response);
    if (status != nullptr) {
      this->terminate(*status);
      return;
    }
    this->StartRead(&this->response);
  }

  void OnDone(const grpc::Status &status) override
  {
    this->terminate(status);
    this->doneCallback();
  }

  bool isDone() {
    return this->done;
  }

  virtual std::unique_ptr<grpc::Status> readResponse(const Response &response) = 0;
  virtual void doneCallback() {};
};

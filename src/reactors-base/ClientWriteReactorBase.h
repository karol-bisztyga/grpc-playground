#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

template<class Request, class Response>
class ClientWriteReactorBase : public grpc::ClientWriteReactor<Request>
{
  bool done = false;
  bool initialized = 0;
  Request request;

public:
  Response response;
  grpc::ClientContext context;
  grpc::Status status;

  void nextWrite()
  {
    std::unique_ptr<grpc::Status> status;
    try {
      status = this->prepareRequest(this->request);
    } catch(std::runtime_error &e) {
      status = std::make_unique<grpc::Status>(grpc::StatusCode::INTERNAL, e.what());
    }
    if (status != nullptr) {
      this->terminate(*status);
      return;
    }
    this->StartWrite(&this->request);
    if (!this->initialized)
    {
      this->StartCall();
      this->initialized = true;
    }
  }

  void OnWriteDone(bool ok) override
  {
    if (!ok)
    {
      this->terminate(grpc::Status(grpc::StatusCode::UNKNOWN, "write error"));
      return;
    }
    this->nextWrite();
  }

  void terminate(const grpc::Status &status)
  {
    this->status = status;
    if (this->done) {
      return;
    }
    this->done = true;
    this->StartWritesDone();
  }

  bool isDone()
  {
    return this->done;
  }

  void OnDone(const grpc::Status &status) override
  {
    this->terminate(status);
    this->doneCallback();
  }

  virtual std::unique_ptr<grpc::Status> prepareRequest(Request &request) = 0;
  virtual void doneCallback() {}
};

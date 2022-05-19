#pragma once

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <functional>

template <class Request, class Response>
class ClientReadReactorBase : public grpc::ClientReadReactor<Response>
{
  Response response;
  bool done = false;

  void terminate(const grpc::Status &status)
  {
    this->status = status;
    if (this->done) {
      return;
    }
    this->done = true;
  }

protected:
  grpc::Status status;

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
    try {
      std::unique_ptr<grpc::Status> status = this->readResponse(this->response);
      if (status != nullptr) {
        this->terminate(*status);
        return;
      }
    } catch(std::runtime_error &e) {
      this->terminate(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
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

#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <memory>

template <class Request, class Response>
class WriteReactorBase : public grpc::ServerWriteReactor<Response>
{
  Response response;
protected:
  // this is a const ref since it's not meant to be modified
  const Request &request;
public:
  WriteReactorBase(const Request *request) : request(*request) {
    // we cannot call this->NextWrite() here because it's going to call it on
    // the base class, not derived leading to the runtime error of calling
    // a pure virtual function
    // NextWrite has to be exposed as a public function and called explicitly
    // to initialize writing
  }

  virtual void NextWrite()
  {
    std::unique_ptr<grpc::Status> status = this->writeResponse(&this->response);
    if (status != nullptr) {
      this->Finish(*status);
      return;
    }
    this->StartWrite(&this->response);
  }

  void OnDone() override
  {
    delete this;
  }

  void OnWriteDone(bool ok) override
  {
    if (!ok)
    {
      this->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "writing error"));
      return;
    }
    try
    {
      this->NextWrite();
    }
    catch (std::runtime_error &e)
    {
      this->Finish(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
    }
  }

  virtual std::unique_ptr<grpc::Status> writeResponse(Response *response) = 0;
};
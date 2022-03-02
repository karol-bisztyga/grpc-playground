#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <memory>

template <class Request, class Response>
class ReadReactorBase : public grpc::ServerReadReactor<Request>
{
  Request request;
protected:
  Response *response;
public:
  ReadReactorBase(Response *response) : response(response)
  {
    this->StartRead(&this->request);
  }

  void OnDone() override
  {
    delete this;
  }
  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      this->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "reading error"));
      return;
    }
    try
    {
      std::unique_ptr<grpc::Status> status = this->readRequest(this->request);
      if (status != nullptr)
      {
        this->Finish(*status);
        return;
      }
    }
    catch (std::runtime_error &e)
    {
      this->Finish(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
      return;
    }
    this->StartRead(&this->request);
  }

  virtual std::unique_ptr<grpc::Status> readRequest(Request request) = 0;
};
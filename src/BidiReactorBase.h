#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <memory>

template <class Request, class Response>
class BidiReactorBase : public grpc::ServerBidiReactor<Request, Response> {
  Request request;
  Response response;

  void finish(grpc::Status status = grpc::Status::OK);

public:
  BidiReactorBase();

  void OnDone() override;
  void OnReadDone(bool ok) override;
  void OnWriteDone(bool ok) override;

  virtual std::unique_ptr<grpc::Status> handleRequest(Request request, Response *response) = 0;
};

template <class Request, class Response>
void BidiReactorBase<Request, Response>::finish(grpc::Status status)
{
  this->Finish(status);
}

template <class Request, class Response>
BidiReactorBase<Request, Response>::BidiReactorBase()
{
  this->StartRead(&this->request);
}

template <class Request, class Response>
void BidiReactorBase<Request, Response>::OnDone() {
  delete this;
}

template <class Request, class Response>
void BidiReactorBase<Request, Response>::OnReadDone(bool ok) {
  if (!ok) {
    this->finish(grpc::Status(grpc::StatusCode::INTERNAL, "reading error"));
    return;
  }
  try {
    std::unique_ptr<grpc::Status> status = this->handleRequest(this->request, &this->response);
    if (status != nullptr) {
      this->finish(*status);
      return;
    }
    this->StartWrite(&this->response);
  } catch (std::runtime_error &e) {
    this->finish(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
  }
}

template <class Request, class Response>
void BidiReactorBase<Request, Response>::OnWriteDone(bool ok)
{
  if (!ok) {
    std::cout << "Server write failed" << std::endl;
    return;
  }
  this->StartRead(&this->request);
}

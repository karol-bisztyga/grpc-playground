#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <memory>

template <class Request, class Response>
class ReactorBase : public grpc::ServerBidiReactor<Request, Response> {
  Request request;
  Response response;

  void finish(grpc::Status status = grpc::Status::OK);

public:
  ReactorBase();

  void OnDone() override;
  void OnReadDone(bool ok) override;
  void OnWriteDone(bool ok) override;

  virtual std::unique_ptr<grpc::Status> handleRequest(Request request, Response *response) = 0;
};

template <class Request, class Response>
void ReactorBase<Request, Response>::finish(grpc::Status status) {
  this->Finish(status);
}

template <class Request, class Response>
ReactorBase<Request, Response>::ReactorBase() {
  this->StartRead(&this->request);
}

template <class Request, class Response>
void ReactorBase<Request, Response>::OnDone() {
  delete this;
}

template <class Request, class Response>
void ReactorBase<Request, Response>::OnReadDone(bool ok) {
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
void ReactorBase<Request, Response>::OnWriteDone(bool ok) {
  if (!ok) {
    std::cout << "Server write failed" << std::endl;
    return;
  }
  this->StartRead(&this->request);
}

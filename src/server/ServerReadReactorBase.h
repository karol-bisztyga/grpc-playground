#pragma once

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <memory>

template <class Request, class Response>
class ServerReadReactorBase : public grpc::ServerReadReactor<Request>
{
  Request request;
protected:
  Response *response;
public:
  ServerReadReactorBase(Response *response);

  void OnDone() override;
  void OnReadDone(bool ok) override;

  void terminate(grpc::Status status);

  virtual std::unique_ptr<grpc::Status> readRequest(Request request) = 0;
  virtual void initialize(){};
  virtual void doneCallback(){};
};

template <class Request, class Response>
ServerReadReactorBase<Request, Response>::ServerReadReactorBase(Response *response) : response(response)
{
  this->initialize();
  this->StartRead(&this->request);
}

template <class Request, class Response>
void ServerReadReactorBase<Request, Response>::OnDone() {
  delete this;
}

template <class Request, class Response>
void ServerReadReactorBase<Request, Response>::OnReadDone(bool ok) {
  if (!ok) {
    this->terminate(grpc::Status(grpc::StatusCode::INTERNAL, "reading error"));
    return;
  }
  try {
    std::unique_ptr<grpc::Status> status = this->readRequest(this->request);
    if (status != nullptr) {
      this->terminate(*status);
      return;
    }
  }
  catch (std::runtime_error &e) {
    this->terminate(grpc::Status(grpc::StatusCode::INTERNAL, e.what()));
    return;
  }
  this->StartRead(&this->request);
}

template <class Request, class Response>
void ServerReadReactorBase<Request, Response>::terminate(grpc::Status status) {
  this->doneCallback();
  this->Finish(status);
}
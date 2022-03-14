#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel)
    : stub(blob::BlobService::NewStub(channel)) {}

void Client::put(const std::string &reverseIndex, const std::string &hash, const std::string &data)
{
  this->putReactor.reset(new PutReactor(reverseIndex, hash, data, &this->persist));
  this->stub->async()->Put(&this->putReactor->context, &(*this->putReactor));
  this->putReactor->nextWrite();
}

void Client::get(const std::string &reverseIndex) {
  this->getReactor.reset(new GetReactor());
  this->getReactor->request.set_holder(reverseIndex);
  this->stub->async()->Get(&this->getReactor->context, &this->getReactor->request, &(*this->getReactor));
  this->getReactor->start();
}

bool Client::remove(const std::string &reverseIndex) {
  grpc::ClientContext context;
  blob::RemoveRequest request;
  google::protobuf::Empty response;

  request.set_holder(reverseIndex);

  grpc::Status status = this->stub->Remove(&context, request, &response);
  if (!status.ok()) {
    std::cout << "an error ocurred: " << status.error_message() << std::endl;
    return false;
  }
  std::cout << "done removing " << reverseIndex << std::endl;
  return true;
}

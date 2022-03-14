#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel)
    : stub(blob::BlobService::NewStub(channel)) {}

void Client::put(const std::string &reverseIndex, const std::string &hash, const std::string &data) {
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

void Client::remove(const std::string &reverseIndex) {
  this->removeReactor.reset(new RemoveReactor(&this->persist));
  this->removeReactor->request.set_holder(reverseIndex);
  this->stub->async()->Remove(&this->removeReactor->context, &this->removeReactor->request, &this->removeReactor->response, &(*this->removeReactor));
  this->removeReactor->StartCall();
}

bool Client::reactorActive()
{
  if (this->getReactor && !this->getReactor->isDone())
  {
    return true;
  }
  if (this->putReactor && !this->putReactor->isDone())
  {
    return true;
  }
  if (this->removeReactor && !this->removeReactor->isDone())
  {
    return true;
  }
  return false;
}
#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel)
    : stub(backup::BackupService::NewStub(channel)) {}

void Client::createNewBackup() {
  this->createNewBackupReactor.reset(new CreateNewBackupReactor(randomNumber(3,6)));
  this->stub->async()->CreateNewBackup(&this->createNewBackupReactor->context, &(*this->createNewBackupReactor));
  this->createNewBackupReactor->nextWrite();
}

void Client::sendLog() {
  this->sendLogReactor.reset(new SendLogReactor(randomNumber(3,6)));
  this->stub->async()->SendLog(&this->sendLogReactor->context, &this->sendLogReactor->response, &(*this->sendLogReactor));
  this->sendLogReactor->nextWrite();
}

bool Client::reactorActive()
{
  if (this->createNewBackupReactor && !this->createNewBackupReactor->isDone())
  {
    return true;
  }
  if (this->sendLogReactor && !this->sendLogReactor->isDone())
  {
    return true;
  }
  return false;
}

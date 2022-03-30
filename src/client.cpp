#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID)
    : stub(backup::BackupService::NewStub(channel)), userID(userID) {}

void Client::createNewBackup() {
  this->createNewBackupReactor.reset(new CreateNewBackupReactor(this->userID, randomNumber(3,6)));
  this->stub->async()->CreateNewBackup(&this->createNewBackupReactor->context, &(*this->createNewBackupReactor));
  this->createNewBackupReactor->nextWrite();
}

void Client::sendLog() {
  this->sendLogReactor.reset(new SendLogReactor(this->userID, 1));
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

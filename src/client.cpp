#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID)
    : stub(backup::BackupService::NewStub(channel)), userID(userID) {}

void Client::createNewBackup() {
  this->createNewBackupReactor.reset(new CreateNewBackupReactor(this->userID, randomNumber(3, 6), this->setLasBackupIDCallback));
  this->stub->async()->CreateNewBackup(&this->createNewBackupReactor->context, &(*this->createNewBackupReactor));
  this->createNewBackupReactor->nextWrite();
}

void Client::sendLog() {
  if (this->lastBackupID.empty()) {
    std::cout << "trying to send log while there's no backup, aborting" << std::endl;
    return;
  }
  this->sendLogReactor.reset(new SendLogReactor(this->userID, 1, this->lastBackupID));
  this->stub->async()->SendLog(&this->sendLogReactor->context, &this->sendLogReactor->response, &(*this->sendLogReactor));
  this->sendLogReactor->nextWrite();
}

void Client::recoverBackupKey() {
  throw std::runtime_error("unimplemented");
}

void Client::pullBackup() {
  this->pullBackupReactor.reset(new PullBackupReactor());
  this->pullBackupReactor->request.set_userid(this->userID);
  this->pullBackupReactor->request.set_backupid(this->lastBackupID);
  this->stub->async()->PullBackup(&this->pullBackupReactor->context, &this->pullBackupReactor->request, &(*this->pullBackupReactor));
  this->pullBackupReactor->start();
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

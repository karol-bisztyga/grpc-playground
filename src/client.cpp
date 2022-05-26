#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID)
    : stub(backup::BackupService::NewStub(channel)), userID(userID) {}

void Client::createNewBackup() {
  this->createNewBackupReactor.reset(new CreateNewBackupReactor(this->userID, this->deviceID, randomNumber(3, 6), this->setLastBackupIDCallback));
  this->stub->async()->CreateNewBackup(&this->createNewBackupReactor->context, &(*this->createNewBackupReactor));
  this->createNewBackupReactor->nextWrite();
}

void Client::sendLog() {
  if (this->lastBackupID.empty()) {
    std::cout << "trying to send log while there's no backup, aborting" << std::endl;
    return;
  }
  this->sendLogReactor.reset(new SendLogReactor(this->userID, 1, this->lastBackupID, this->setLastLogIDCallback));
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

void Client::addAttachments(bool isForLog)
{
  if (this->lastBackupID.empty())
  {
    std::cout << "trying to send attachment while there's no backup, aborting" << std::endl;
    return;
  }
  std::string logID = "";
  if (isForLog) {
    logID = this->lastLogID;
  }
  std::string holders;
  std::cout << "adding attachments: " << std::endl;
  for (size_t i = 0, l = randomNumber(2,5); i < l; ++i) {
    std::string holder = randomString();
    holders += holder;
    holders += ATTACHMENT_DELIMITER;
    std::cout << " - " << holder << std::endl;
  }

  this->addAttachmentsReactor.reset(new AddAttachmentsReactor());
  this->addAttachmentsReactor->request.set_userid(this->userID);
  this->addAttachmentsReactor->request.set_backupid(this->lastBackupID);
  this->addAttachmentsReactor->request.set_logid(logID);
  this->addAttachmentsReactor->request.set_holders(holders);
  this->stub->async()->AddAttachments(&this->addAttachmentsReactor->context, &this->addAttachmentsReactor->request, &this->addAttachmentsReactor->response, &(*this->addAttachmentsReactor));
  this->addAttachmentsReactor->StartCall();
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

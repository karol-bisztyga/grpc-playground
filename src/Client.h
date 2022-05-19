#pragma once

#include "SendLogReactor.h"
#include "CreateNewBackupReactor.h"
#include "PullBackupReactor.h"
#include "AddAttachmentReactor.h"
#include "Tools.h"

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>

enum class AttachmentParentType {
  BACKUP = 0,
  LOG = 1,
};

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;
  const std::string userID;
  std::string lastBackupID = "";
  std::string lastLogID = "";

  const std::function<void(const std::string &)> setLastBackupIDCallback = [this](const std::string &backupID)
  {
    this->lastBackupID = backupID;
  };

  const std::function<void(const std::string &)> setLastLogIDCallback = [this](const std::string &logID)
  {
    this->lastLogID = logID;
  };

public:
  Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;
  std::unique_ptr<SendLogReactor> sendLogReactor;
  std::unique_ptr<PullBackupReactor> pullBackupReactor;
  std::unique_ptr<AddAttachmentReactor> addAttachmentReactor;

  void createNewBackup();
  void sendLog();
  void recoverBackupKey();
  void pullBackup();
  void addAttachment(bool isForLog);

  bool reactorActive();

  std::string getCurrentBackupID () {
    return this->lastBackupID;
  }
  std::string getCurrentLogID() {
    return this->lastLogID;
  }
};

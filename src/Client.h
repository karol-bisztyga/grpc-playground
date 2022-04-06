#pragma once

#include "SendLogReactor.h"
#include "CreateNewBackupReactor.h"
#include "PullBackupReactor.h"
#include "Tools.h"

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;
  const std::string userID;
  std::string lastBackupID = "zJKLR74t1d1iHRKefeCo";

  const std::function<void(const std::string&)> setLasBackupIDCallback = [this](const std::string &backupID){
    this->lastBackupID = backupID;
  };

public:
  Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;
  std::unique_ptr<SendLogReactor> sendLogReactor;
  std::unique_ptr<PullBackupReactor> pullBackupReactor;

  void createNewBackup();
  void sendLog();
  void recoverBackupKey();
  void pullBackup();

  bool reactorActive();

  std::string getCurrentBackupID () {
    return this->lastBackupID;
  }
};

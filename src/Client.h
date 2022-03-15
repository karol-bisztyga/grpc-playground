#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include <grpcpp/grpcpp.h>

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include "ClientBidiReactorBase.h"
#include "ClientReadReactorBase.h"
#include "Tools.h"

enum class CreateNewBackupState {
  //...
};

class CreateNewBackupReactor : public ClientBidiReactorBase<backup::CreateNewBackupRequest, backup::CreateNewBackupResponse>
{
  // CreateNewBackupState state = CreateNewBackupState::;
public:
  std::unique_ptr<grpc::Status> prepareRequest(backup::CreateNewBackupRequest &request, std::shared_ptr<backup::CreateNewBackupResponse> previousResponse) override
  {
    return nullptr;
  }
};

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;

public:
  Client(std::shared_ptr<grpc::Channel> channel);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;

  void createNewBackup();
  void sendLog();
  void recoverBackupKey();
  void pullBackup();

  bool reactorActive();
};

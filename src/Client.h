#pragma once

#include <grpcpp/grpcpp.h>

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <string>
#include <memory>

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;
  const std::string id;
  const std::string deviceToken;
public:
  Client(std::shared_ptr<grpc::Channel> channel, std::string id);

  void resetKey(const std::string newKey, const std::vector<std::string> newCompact);
  void sendLog(const std::string data);
  void pullBackupKey(const std::string pakeKey);
  void pullCompact();
};

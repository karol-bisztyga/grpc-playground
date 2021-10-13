#pragma once

#include <grpcpp/grpcpp.h>

#include "../_generated/tunnelbroker.pb.h"
#include "../_generated/tunnelbroker.grpc.pb.h"

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <string>
#include <memory>

class TunnelBrokerClient
{
  std::unique_ptr<tunnelbroker::TunnelBrokerService::Stub> tbStub_;
  std::unique_ptr<tunnelbroker::BackupService::Stub> backupStub_;
  const std::string id;
  const std::string deviceToken;
public:
  TunnelBrokerClient(std::shared_ptr<grpc::Channel> channel, std::string id, std::string deviceToken);
  tunnelbroker::CheckResponseType checkIfPrimaryDeviceOnline();
  bool becomeNewPrimaryDevice();
  void sendPong();

  void sendLog(const std::string data);
  void resetLog();
  void restoreBackup();
};
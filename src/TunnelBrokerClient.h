#pragma once

#include <grpcpp/grpcpp.h>

#include "../_generated/tunnelbroker.pb.h"
#include "../_generated/tunnelbroker.grpc.pb.h"

#include <string>
#include <memory>

class TunnelBrokerClient
{
  std::unique_ptr<tunnelbroker::TunnelBrokerService::Stub> stub_;
  const std::string id;
  const std::string deviceToken;
public:
  TunnelBrokerClient(std::shared_ptr<grpc::Channel> channel, std::string id, std::string deviceToken);
  tunnelbroker::CheckResponseType checkIfPrimaryDeviceOnline();
  bool becomeNewPrimaryDevice();
  void sendPong();
};
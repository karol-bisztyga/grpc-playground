#pragma once

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include <grpcpp/grpcpp.h>

#include "ClientTalkReactor.h"

#include <vector>
#include <string>
#include <memory>

class ClientWrapper
{
  std::unique_ptr<ClientTalkReactor> reactor;
  std::unique_ptr<inner::InnerService::Stub> stub;
  const int id;
public:
  ClientWrapper(int id, std::shared_ptr<grpc::Channel> channel): stub(inner::InnerService::NewStub(channel)), id(id) {}
  
  void talk(std::vector<std::string> messages) {
    this->reactor.reset(new ClientTalkReactor(id, messages));
    this->stub->async()->TalkBetweenServices(&this->reactor->context, &(*this->reactor));
    this->reactor->start();
  }
};

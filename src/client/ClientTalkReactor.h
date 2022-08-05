#pragma once

#include "../_generated/outer.grpc.pb.h"
#include "../_generated/outer.pb.h"

#include "ClientBidiReactorBase.h"

#include <grpcpp/grpcpp.h>

#include <vector>
#include <string>
#include <memory>

class ClientTalkReactor : public ClientBidiReactorBase<
                                  outer::TalkWithClientRequest,
                                  outer::TalkWithClientResponse>
{
  std::vector<std::string> messages;
  size_t currentIndex = 0;
  const int id;
public:
  ClientTalkReactor(int id, std::vector<std::string> messages) : messages(messages), id(id) {}

  std::unique_ptr<grpc::Status> prepareRequest(outer::TalkWithClientRequest &request, std::shared_ptr<outer::TalkWithClientResponse> previousResponse) override
  {
    if (this->currentIndex >= this->messages.size()) {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    request.set_msg(this->messages[this->currentIndex++]);
    return nullptr;
  }

  void doneCallback() override {
    std::cout << "client DONE [id: " << this->id << "]" << std::endl;
  }
};

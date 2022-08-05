#pragma once

#include "ServiceClient.h"

#include "../_generated/outer.grpc.pb.h"
#include "../_generated/outer.pb.h"

#include "ServerBidiReactorBase.h"

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

class TalkWithClientReactor : public ServerBidiReactorBase<
                                  outer::TalkWithClientRequest,
                                  outer::TalkWithClientResponse> {
  TalkBetweenServicesReactor talkReactor;

  std::mutex reactorStateMutex;
  std::thread clientThread;

  std::condition_variable putDoneCV;
  std::mutex putDoneCVMutex;
public:
  std::unique_ptr<ServerBidiReactorStatus> handleRequest(
      outer::TalkWithClientRequest request,
      outer::TalkWithClientResponse *response) override {
    // we make sure that the client's state is flushed to the main memory
    // as there may be multiple threads from the pool taking over here
    const std::lock_guard<std::mutex> lock(this->reactorStateMutex);
    std::string msg = request.msg();
    if (!this->talkReactor.initialized) {
      this->talkReactor =
          TalkBetweenServicesReactor(&this->putDoneCV);
      this->clientThread =
          ServiceClient::getInstance().talk(this->talkReactor);
    }
    this->talkReactor.scheduleMessage(std::make_unique<std::string>(msg));
    return nullptr;
  }

  void terminateCallback() override {
    const std::lock_guard<std::mutex> lock(this->reactorStateMutex);
    if (!this->talkReactor.initialized) {
      return;
    }
    this->talkReactor.scheduleMessage(std::make_unique<std::string>(""));
    std::unique_lock<std::mutex> lock2(this->putDoneCVMutex);
    this->putDoneCV.wait(lock2);
    if (!this->talkReactor.getStatusHolder()->getStatus().ok()) {
      throw std::runtime_error(
          this->talkReactor.getStatusHolder()->getStatus().error_message());
    }
    this->clientThread.join();
  }
};

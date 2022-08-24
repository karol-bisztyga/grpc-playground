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

  bool putDoneCVReady = false;
  std::condition_variable putDoneCV;
  std::mutex putDoneCVMutex;
public:
  std::unique_ptr<ServerBidiReactorStatus> handleRequest(
      outer::TalkWithClientRequest request,
      outer::TalkWithClientResponse *response) override {
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::handleRequest]" << std::endl;
    // we make sure that the client's state is flushed to the main memory
    // as there may be multiple threads from the pool taking over here
    const std::lock_guard<std::mutex> lock(this->reactorStateMutex);
    std::string msg = request.msg();
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::handleRequest] msg " << msg.size()
              << std::endl;
    if (!this->talkReactor.initialized) {
      std::cout
          << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
          << "]"
          << "[TalkWithClientReactor::handleRequest] initializING talk reactor"
          << std::endl;
      this->talkReactor =
          TalkBetweenServicesReactor(&this->putDoneCV,
          &this->putDoneCVReady, &this->putDoneCVMutex);
      std::cout << "["
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "]"
                << "[TalkWithClientReactor::handleRequest] initializING talk "
                   "reactor2"
                << std::endl;
      this->clientThread =
          ServiceClient::getInstance().talk(this->talkReactor);
      std::cout
          << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
          << "]"
          << "[TalkWithClientReactor::handleRequest] initializED talk reactor"
          << std::endl;
    }
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::handleRequest] schedulING msg "
              << msg.size() << std::endl;
    this->talkReactor.scheduleMessage(std::make_unique<std::string>(msg));
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::handleRequest] schedulED msg "
              << msg.size() << std::endl;
    return nullptr;
  }

  void terminateCallback() override {
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::terminateCallback]" << std::endl;
    const std::lock_guard<std::mutex> lock(this->reactorStateMutex);
    if (!this->talkReactor.initialized) {
      return;
    }
    this->talkReactor.scheduleMessage(std::make_unique<std::string>(""));
    std::unique_lock<std::mutex> lock2(this->putDoneCVMutex);
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::terminateCallback] waitING"
              << std::endl;
    this->putDoneCV.wait(lock2, [this]{ return this->putDoneCVReady; });
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[TalkWithClientReactor::terminateCallback] waitED"
              << std::endl;
    if (!this->talkReactor.getStatusHolder()->getStatus().ok()) {
      throw std::runtime_error(
          this->talkReactor.getStatusHolder()->getStatus().error_message());
    }
    this->clientThread.join();
  }
};
#pragma once

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include "ClientBidiReactorBase.h"
#include "ThreadSafeQueue.h"

#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <memory>
#include <string>
#include <thread>

class TalkBetweenServicesReactor : public ClientBidiReactorBase<
                                       inner::TalkBetweenServicesRequest,
                                       inner::TalkBetweenServicesResponse> {
  ThreadSafeQueue<std::string> messages;

public:
  std::condition_variable *terminationNotifier;
  bool initialized;
  bool *putDoneCVReady;
  std::mutex *putDoneCVMutex;

  TalkBetweenServicesReactor() : initialized(false) {
  }

  TalkBetweenServicesReactor(std::condition_variable *terminationNotifier, bool *putDoneCVReady, std::mutex *putDoneCVMutex)
      : terminationNotifier(terminationNotifier), initialized(true), putDoneCVReady(putDoneCVReady), putDoneCVMutex(putDoneCVMutex) {
  }

  TalkBetweenServicesReactor
  operator=(const TalkBetweenServicesReactor &other) {
    if (this == &other) {
      return *this;
    }
    this->terminationNotifier = other.terminationNotifier;
    this->putDoneCVReady = other.putDoneCVReady;
    this->putDoneCVMutex = other.putDoneCVMutex;
    this->initialized = true;
    return *this;
  }
  TalkBetweenServicesReactor(const TalkBetweenServicesReactor &other) {
    this->terminationNotifier = other.terminationNotifier;
    this->putDoneCVReady = other.putDoneCVReady;
    this->putDoneCVMutex = other.putDoneCVMutex;
    this->initialized = true;
  }

  void scheduleMessage(std::unique_ptr<std::string> msg);
  std::unique_ptr<grpc::Status> prepareRequest(
      inner::TalkBetweenServicesRequest &request,
      std::shared_ptr<inner::TalkBetweenServicesResponse> previousResponse)
      override;
  void doneCallback() override;
};

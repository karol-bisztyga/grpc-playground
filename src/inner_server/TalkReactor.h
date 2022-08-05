#pragma once

#include "ServerBidiReactorBase.h"

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

class TalkReactor : public ServerBidiReactorBase<
                        inner::TalkBetweenServicesRequest,
                        inner::TalkBetweenServicesResponse> {
public:
  std::unique_ptr<ServerBidiReactorStatus> handleRequest(
      inner::TalkBetweenServicesRequest request,
      inner::TalkBetweenServicesResponse *response) override {
    std::string msg = request.msg();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return nullptr;
  }
};

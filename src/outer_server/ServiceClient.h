#pragma once

#include "TalkBetweenServicesReactor.h"

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

class ServiceClient {
  std::shared_ptr<grpc::Channel> channel;

  ServiceClient() {
    std::string targetStr = "localhost:50051";
    this->channel =
        grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials());
  }

public:
  static ServiceClient &getInstance() {
    static ServiceClient instance;
    return instance;
  }

  std::thread talk(TalkBetweenServicesReactor &talkReactor) {
    if (!talkReactor.initialized) {
      throw std::runtime_error(
          "talk reactor is being used but has not been initialized");
    }
    std::thread th([this, &talkReactor]() {
      inner::InnerService::NewStub(this->channel)
          ->async() // this runs on the same thread, why??
          ->TalkBetweenServices(&talkReactor.context, &talkReactor);
      talkReactor.start();
    });
    return th;
  }
};

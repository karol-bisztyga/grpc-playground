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

  // std::thread
  void talk(std::shared_ptr<TalkBetweenServicesReactor> talkReactor) {
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[ServiceClient::talk] etner" << std::endl;
    if (!talkReactor->initialized) {
      throw std::runtime_error(
          "talk reactor is being used but has not been initialized");
    }
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[ServiceClient::talk] etner2" << std::endl;
    // std::thread th([this](std::shared_ptr<TalkBetweenServicesReactor> talkReactor) {
      std::cout << "["
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "]"
                << "[ServiceClient::talk::lambda] startING" << std::endl;
      inner::InnerService::NewStub(this->channel)
          ->async() // this runs on the same thread, why??
          ->TalkBetweenServices(&talkReactor->context, &(*talkReactor));
      talkReactor->start();
      std::cout << "["
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "]"
                << "[ServiceClient::talk::lambda] startED" << std::endl;
    // }, talkReactor);
    // return th;
  }
};

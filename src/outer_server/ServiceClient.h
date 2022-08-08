#pragma once

#include "TalkBetweenServicesReactor.h"

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

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

  std::thread talk(TalkBetweenServicesReactor &talkReactor, std::vector<std::thread> &ths, bool &clientReady, std::mutex &mtx) {
    std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]"
              << "[ServiceClient::talk] enter" << std::endl;
    if (!talkReactor.initialized) {
      throw std::runtime_error(
          "talk reactor is being used but has not been initialized");
    }
    std::thread th([this, &talkReactor, &ths, &mtx, &clientReady]() {
      std::cout << "["
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "]"
                << "[ServiceClient::talk::lambda] startING" << std::endl;
      // inner::InnerService::NewStub(this->channel)
      //     ->async() // this runs on the same thread, why??
      //     ->TalkBetweenServices(&talkReactor.context, &talkReactor);
      // talkReactor.start();
      while (true) {
        std::string msg = talkReactor.messages.dequeue();
        std::cout << "["
                  << std::hash<std::thread::id>{}(std::this_thread::get_id())
                  << "]"
                  << "[ServiceClient::talk::lambda] loop " << msg.size() << std::endl;
        if (msg.empty()) {
          break;
        }
        std::thread ith([msg](){
          std::cout << "["
                  << std::hash<std::thread::id>{}(std::this_thread::get_id())
                  << "]"
                  << "[ServiceClient::talk::lambda] internal begin " << msg.size() << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(100*msg.size()));
          std::cout << "["
                  << std::hash<std::thread::id>{}(std::this_thread::get_id())
                  << "]"
                  << "[ServiceClient::talk::lambda] internal end " << msg.size() << std::endl;
        });
        ths.push_back(std::move(ith));
      }
      {
        std::unique_lock<std::mutex> lock(mtx);
        clientReady = true;
      }
      talkReactor.terminationNotifier->notify_one();
      std::cout << "["
                << std::hash<std::thread::id>{}(std::this_thread::get_id())
                << "]"
                << "[ServiceClient::talk::lambda] terminate" << std::endl;
    });
    return th;
  }
};

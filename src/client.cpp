#include "Tools.h"
#include "Constants.h"
#include "ClientWrapper.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#define MB 1024 * 1024

int main(int argc, char **argv)
{
  std::string targetAddr = LISTEN_ADDRESS + ":" + OUTER_SERVER_PORT;
  auto channel = grpc::CreateChannel(
      targetAddr,
      grpc::InsecureChannelCredentials());
  std::cout << "client start on: " << targetAddr << std::endl;

  int numberOfThreads = std::stoi(std::string(argv[1]));
  std::cout << "client threads: " << numberOfThreads << std::endl;

  std::vector<std::thread> threads;
  std::vector<ClientWrapper> clients;
  for (int i=0; i<numberOfThreads; ++i) {
    clients.push_back(ClientWrapper(i, channel));
  }

  for (int i=0; i<numberOfThreads; ++i) {
    threads.push_back(std::thread([i, &clients](){
      std::cout << "starting client thread " << i << std::endl;
      int numberOfMessages = randomNumber(5,15);
      std::vector<std::string> messages;
      for (int i=0; i<numberOfMessages; ++i) {
        messages.push_back(randomString(20+i));
      }
      clients[i].talk(messages);
    }));
  }

  for (auto &thread : threads) {
    thread.join();
  }


  std::cout << "press enter to terminate" << std::endl;
  std::cin.ignore();

  return 0;
}

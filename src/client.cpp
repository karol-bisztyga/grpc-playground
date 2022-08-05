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
  ClientWrapper client(channel);

  try
  {
    int numberOfMessages = randomNumber(5,15);
    std::vector<std::string> messages;
    for (int i=0; i<numberOfMessages; ++i) {
      messages.push_back(randomString());
    }
    client.talk(messages);
  }
  catch (std::runtime_error &e)
  {
    std::cout << "error: " << e.what() << std::endl;
  }

  std::cout << "press enter to terminate" << std::endl;
  std::cin.ignore();

  return 0;
}

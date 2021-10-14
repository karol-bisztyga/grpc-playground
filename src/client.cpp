#include "TunnelBrokerClient.h"

#include <grpcpp/grpcpp.h>

#include "../_generated/tunnelbroker.pb.h"
#include "../_generated/tunnelbroker.grpc.pb.h"

#include <iostream>
#include <memory>
#include <string>

#include <chrono>

using namespace std::chrono;

// this is a simulation of a device token
// I figured we can use a timestamp in milliseconds as a unique id for a process
// I'm going to assume it's not possible to launch multiple processes
// at the same very millisecond
long long generateUID()
{
  milliseconds ms = duration_cast<milliseconds>(
      system_clock::now().time_since_epoch());
  return ms.count();
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "please provide an argument with a client id" << std::endl;
    return 1;
  }
  std::string id = std::string(argv[1]);
  std::string deviceToken = std::to_string(generateUID());
  std::cout << "client start" << std::endl;
  std::cout << "id           : " << id << std::endl;
  std::cout << "device token : " << deviceToken << std::endl;
  std::string target_str = "localhost:50051";

  TunnelBrokerClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()),
      id,
      deviceToken);

  char option = '?';
  while (option != 'e')
  {
    std::string options = "cpse";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[c] check if primary device is online" << std::endl;
    std::cout << "[p] become a new primary device" << std::endl;
    std::cout << "[s] send pong" << std::endl;
    std::cout << "[e] exit" << std::endl;
    std::cin >> option;
    if (options.find(option) == std::string::npos)
    {
      std::cout << "invalid command [" << option << "], skipping" << std::endl;
      continue;
    }
    try
    {
      switch (option)
      {
      case 'c':
      {
        tunnelbroker::CheckResponseType checkResponse = client.checkIfPrimaryDeviceOnline();
        std::cout << "check primary device response: " << tunnelbroker::CheckResponseType_Name(checkResponse) << std::endl;
        break;
      }
      case 'p':
      {
        bool success = client.becomeNewPrimaryDevice();
        std::cout << "trying to become a new primary device... ";
        if (success)
        {
          std::cout << "success";
        }
        else
        {
          std::cout << "failed";
        }
        std::cout << std::endl;
        break;
      }
      case 's':
      {
        std::cout << "sending pong... ";
        client.sendPong();
        std::cout << "sent!" << std::endl;
        break;
      }
      }
    }
    catch (std::runtime_error &e)
    {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

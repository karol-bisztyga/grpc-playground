#include "TunnelBrokerClient.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <random>
#include <string>

std::string randomString()
{
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  std::random_device rd;
  std::mt19937 generator(rd());

  std::shuffle(str.begin(), str.end(), generator);

  return str.substr(0, 32);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "please provide an argument with a client id" << std::endl;
    return 1;
  }
  std::string id = std::string(argv[1]);
  std::cout << "client start" << std::endl;
  std::cout << "id           : " << id << std::endl;
  std::string target_str = "localhost:50051";

  Client client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()),
      id);

  char option = '?';
  while (option != 'e')
  {
    std::string options = "srbe";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[s] send log" << std::endl;
    std::cout << "[r] reset log" << std::endl;
    std::cout << "[b] restore backup" << std::endl;
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
      case 's':
      {
        const std::string data = randomString() + " ";
        std::cout << "sending log: [" << data << "]" << std::endl;
        client.sendLog(data);
        break;
      }
      case 'r':
      {
        std::cout << "reseting log... " << std::endl;
        client.resetLog();
        break;
      }
      case 'b':
      {
        std::cout << "restoring backup... " << std::endl;
        client.restoreBackup();
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

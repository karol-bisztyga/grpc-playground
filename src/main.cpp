#include "Client.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <random>
#include <string>

int randomNumber(const int from, const int to)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(from, to);

  return dist(mt);
}

std::string randomString(size_t size = 20)
{
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  std::string result;

  for (size_t i = 0; i < size; ++i) {
    result += str[randomNumber(0, str.size() - 1)];
  }

  return result;
}

int main(int argc, char **argv)
{
  if (argc != 2 && argc != 3)
  {
    std::cout << "please provide an argument with a client id" << std::endl;
    std::cout << "you can also specify a port as an optional argument" << std::endl;
    return 1;
  }
  std::string id = std::string(argv[1]);
  std::string port = "50051";
  if (argc == 3) {
    port = std::string(argv[2]);
  }
  std::cout << "client start, target port is " << port << std::endl;
  std::cout << "id           : " << id << std::endl;
  std::string target_str = "localhost:" + port;

  Client client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()),
      id);

  char option = '?';
  while (option != 'e')
  {
    std::string options = "rskce";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[r] reset key" << std::endl;
    std::cout << "[s] send log" << std::endl;
    std::cout << "[k] pull backup key" << std::endl;
    std::cout << "[c] pull compact" << std::endl;
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
      case 'r':
      {
        std::cout << "resetting key" << std::endl;
        std::vector<std::string> newCompact;
        const size_t nCompacts = randomNumber(3, 7);
        for (size_t i = 0; i < nCompacts; ++i)
        {
          std::string chunk = randomString();
          std::cout << "adding compact chunk [" << chunk << "]" << std::endl;
          newCompact.push_back(chunk);
        }
        client.resetKey(randomString(), newCompact);
        break;
      }
      case 's':
      {
        std::cout << "sending log... " << std::endl;
        client.sendLog(randomString());
        break;
      }
      case 'k':
      {
        std::cout << "pulling backup key... " << std::endl;
        client.pullBackupKey(randomString());
        break;
      }
      case 'c':
      {
        std::cout << "pulling compact... " << std::endl;
        client.pullCompact();
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

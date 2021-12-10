#include "Tools.h"
#include "Client.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
  std::string port = "50053";
  std::cout << "you can specify a port as an optional argument(default is " << port << ")" << std::endl;
  if (argc >= 2) {
    port = std::string(argv[1]);
  }
  std::cout << "client start, target port is " << port << std::endl;
  std::string target_str = "localhost:" + port;

  Client client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  char option = '?';
  while (option != 'e')
  {
    std::string options = "gpre";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[g] get" << std::endl;
    std::cout << "[p] put" << std::endl;
    std::cout << "[r] remove" << std::endl;
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
      case 'g':
      {
        std::cout << "get data" << std::endl;
        std::function<void(std::string)> callback = [](std::string chunk)
        {
          std::cout << "read chunk(clb) [" << chunk << "]" << std::endl;
        };
        std::string hash = randomString(); // todo handle hashes
        client.get(hash, callback);
        break;
      }
      case 'p':
      {
        std::cout << "put data" << std::endl;
        const size_t nChunks = 10;
        size_t chunkCounter = 0;
        std::function<std::string()> dataChunksObtainer = [nChunks, &chunkCounter]()
        {
          // todo improve those chunks
          if (chunkCounter >= nChunks)
          {
            return std::string("");
          }
          ++chunkCounter;
          return randomString(100);
        };
        client.put(randomString(), dataChunksObtainer);
        break;
      }
      case 'r':
      {
        std::cout << "remove data" << std::endl;
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

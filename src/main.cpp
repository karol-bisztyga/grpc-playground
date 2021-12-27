#include "Tools.h"
#include "Client.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>

#define MB 1024 * 1024

void put(Client &client, const size_t dataChunkSize, size_t nChunks=0, char forcedFirstChar=0) {
    std::cout << "put data" << std::endl;

    const std::string reverseIndex = randomString();
    const std::string hash = std::to_string(currentTimestamp());

    if (client.persist.find(reverseIndex) != client.persist.end()) {
      std::cout << "reverse index already exists, aborting: " << reverseIndex << std::endl;
      return;
    }

    if (nChunks == 0) {
      nChunks = randomNumber(5, 10);
    }
    size_t chunkCounter = 0;
    std::function<std::string()> dataChunksObtainer = [nChunks, &chunkCounter, dataChunkSize, forcedFirstChar]()
    {
      if (chunkCounter++ >= nChunks)
      {
        return std::string("");
      }
      std::cout << "updating chunk " << chunkCounter << std::endl;
      if (forcedFirstChar == 0)
      {
        return randomString(dataChunkSize);
      }
      std::string result;
      result.resize(dataChunkSize);
      memset(result.data(), forcedFirstChar+chunkCounter, dataChunkSize);
      return result;
    };
    client.put(reverseIndex, hash, dataChunksObtainer);
    client.persist.insert(std::pair<std::string, std::string>(reverseIndex, hash));
}

int main(int argc, char **argv)
{
  std::string port = "50053";
  std::cout << "you can specify a port as an optional argument(default is " << port << ")" << std::endl;
  if (argc >= 2)
  {
    port = std::string(argv[1]);
  }
  std::cout << "client start, target port is " << port << std::endl;
  std::string target_str = "localhost:" + port;

  Client client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  char option = '?';
  while (option != 'e')
  {
    std::string options = "gpPre";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[g] get" << std::endl;
    std::cout << "[p] put" << std::endl;
    std::cout << "[P] put a big chunk of data - over 5MB" << std::endl;
    std::cout << "[r] remove" << std::endl;
    std::cout << "[e] exit" << std::endl;
    std::cout << std::endl << "current persist[rev index/hash]:" << std::endl;
    if (!client.persist.size()) {
      std::cout << "(empty)";
    } else {
      for (auto it = client.persist.begin(); it != client.persist.end(); it++) {
        std::cout << it->first << "/" << it->second << std::endl;
      }
    }
    std::cout << std::endl;
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
        std::cout << "get data, please enter a desired reverse index" << std::endl;
        std::string reverseIndex;
        std::cin >> reverseIndex;
        
        std::function<void(std::string)> callback = [](std::string chunk)
        {
          std::cout << "read chunk(clb) [" << chunk << "]" << std::endl;
        };
        client.get(reverseIndex, callback);
        break;
      }
      case 'p':
      {
        put(client, 100);
        break;
      }
      case 'P':
      {
        put(client, MB, 8, 64);
        break;
      }
      case 'r':
      {
        std::cout << "remove data, please enter a desired reverse index" << std::endl;
        std::string reverseIndex;
        std::cin >> reverseIndex;
        if (client.remove(reverseIndex)) {
          client.persist.erase(reverseIndex);
        }
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

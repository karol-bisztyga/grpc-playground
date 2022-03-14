#include "Tools.h"
#include "Client.h"

#include <grpcpp/grpcpp.h>
#include <openssl/sha.h>

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>

#define MB 1024 * 1024

void put(Client &client, size_t dataSize = 0, char forcedFirstChar = 0)
{
  std::cout << "put data" << std::endl;

  const std::string reverseIndex = randomString();

  if (client.persist.find(reverseIndex) != client.persist.end())
  {
    std::cout << "reverse index already exists, aborting: " << reverseIndex << std::endl;
    return;
  }

  if (dataSize == 0)
  {
    dataSize = randomNumber(100, 500);
  }

  std::string data;
  if (forcedFirstChar)
  {
    data.resize(dataSize);
    memset(data.data(), forcedFirstChar, dataSize);
  }
  else
  {
    data = randomString(dataSize);
  }

  unsigned char hash[SHA512_DIGEST_LENGTH];
  SHA512(
      (const unsigned char *)data.data(),
      data.size(),
      hash);

  std::ostringstream hashStream;
  for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
  {
    hashStream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase
               << (int)hash[i];
  }
  client.put(reverseIndex, hashStream.str(), data);
}

enum class Mode
{
  LOCALHOST = 1,
  LB = 2,
};

int main(int argc, char **argv)
{
  std::string targetStr;
  std::unique_ptr<Client> client;

  Mode mode = Mode::LOCALHOST;
  switch(mode)
  {
  case Mode::LOCALHOST:
  {
    targetStr = "localhost:50053";
    client = std::make_unique<Client>(grpc::CreateChannel(
        targetStr,
        grpc::InsecureChannelCredentials()));
    break;
  }
  case Mode::LB:
  {
    targetStr = "blob.prod.comm.dev:50053";
    client = std::make_unique<Client>(grpc::CreateChannel(targetStr, grpc::SslCredentials(grpc::SslCredentialsOptions())));
    break;
  }
  }

  std::cout << "client start on: " << targetStr << std::endl;

  char option = '?';
  while (option != 'e')
  {
    std::string options = "gpPrea";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[g] get" << std::endl;
    std::cout << "[p] put" << std::endl;
    std::cout << "[P] put a big chunk of data - over 5MB" << std::endl;
    std::cout << "[r] remove" << std::endl;
    std::cout << "[a] remove ALL" << std::endl;
    std::cout << "[e] exit" << std::endl;
    std::cout << std::endl
              << "current persist[rev index/hash]:" << std::endl;
    if (!client->persist.size())
    {
      std::cout << "(empty)";
    }
    else
    {
      for (auto it = client->persist.begin(); it != client->persist.end(); it++)
      {
        std::cout << it->first << " / " << it->second << std::endl;
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

        client->get(reverseIndex);
        break;
      }
      case 'p':
      {
        put(*client);
        break;
      }
      case 'P':
      {
        put(*client, MB * 20, 66);
        break;
      }
      case 'r':
      {
        std::cout << "remove data, please enter a desired reverse index" << std::endl;
        std::string reverseIndex;
        std::cin >> reverseIndex;
        if (client->remove(reverseIndex))
        {
          client->persist.erase(reverseIndex);
        }
        break;
      }
      case 'a':
      {
        while(!client->persist.empty()) {
          auto item = client->persist.begin();
          std::cout << "removing " << item->first << std::endl;
          if (client->remove(item->first))
          {
            client->persist.erase(item->first);
          }
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

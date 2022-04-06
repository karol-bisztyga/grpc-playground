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

// void put(Client &client, size_t dataSize = 0, char forcedFirstChar = 0)
// {
//   std::cout << "put data" << std::endl;
//   if (dataSize == 0) {
//     dataSize = randomNumber(100, 500);
//   }
//   std::string data;
//   if (forcedFirstChar)
//   {
//     data.resize(dataSize);
//     memset(data.data(), forcedFirstChar, dataSize);
//   }
//   else
//   {
//     data = randomString(dataSize);
//   }

//   unsigned char hash[SHA512_DIGEST_LENGTH];
//   SHA512(
//       (const unsigned char *)data.data(),
//       data.size(),
//       hash);

//   std::ostringstream hashStream;
//   for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
//   {
//     hashStream << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase
//                << (int)hash[i];
//   }
//   std::cout << "trying to put: [size: " << data.size() << "]" << std::endl;//[" << data << "]" << std::endl;
//   client.put(reverseIndex, hashStream.str(), data);
// }

enum class Mode
{
  LOCALHOST = 1,
  LB = 2,
};

int main(int argc, char **argv)
{
  std::string targetStr;
  std::unique_ptr<Client> client;
  const std::string userID = "3015";//std::to_string(randomNumber(1000,10000));

  Mode mode = Mode::LOCALHOST;
  switch(mode)
  {
  case Mode::LOCALHOST:
  {
    targetStr = "localhost:50052";
    client = std::make_unique<Client>(grpc::CreateChannel(
        targetStr,
        grpc::InsecureChannelCredentials()), userID);
    break;
  }
  case Mode::LB:
  {
    targetStr = "backup.prod.comm.dev:50052";
    client = std::make_unique<Client>(grpc::CreateChannel(targetStr, grpc::SslCredentials(grpc::SslCredentialsOptions())), userID);
    break;
  }
  }

  std::cout << "client(id: " << userID << ") start on: " << targetStr << std::endl;

  char option = '?';
  while (option != 'e')
  {
    std::string options = "nlrpe";
    std::cout << " - current backup id: [" << client->getCurrentBackupID() << "]" << std::endl;
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[n] new backup" << std::endl;
    std::cout << "[l] send log" << std::endl;
    std::cout << "[r] recover backup key" << std::endl;
    std::cout << "[p] pull backup" << std::endl;
    std::cout << "[e] exit" << std::endl;
    std::cout << std::endl;
    std::cin >> option;
    if (options.find(option) == std::string::npos)
    {
      std::cout << "invalid command [" << option << "], skipping" << std::endl;
      continue;
    }
    if (client->reactorActive()) {
      std::cout << "a reactor is working, please wait" << std::endl;
      continue;
    }
    try
    {
      switch (option)
      {
      case 'n':
      {
        client->createNewBackup();
        break;
      }
      case 'l':
      {
        client->sendLog();
        break;
      }
      case 'r':
      {
        break;
      }
      case 'p':
      {
        client->pullBackup();
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

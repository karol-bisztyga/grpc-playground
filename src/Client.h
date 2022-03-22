#pragma once

#include "ClientBidiReactorBase.h"
#include "ClientWriteReactorBase.h"
#include "ClientReadReactorBase.h"
#include "Tools.h"

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

/*
SEND_KEY_ENTROPY
client => keyEntropy => server
SEND_DATA_HASH
client => dataHash => server
SEND_CHUNKS
client => data chunk 0 => server
client => data chunk 1 => server
client => data chunk ... => server
client => data chunk n => server

end connection
*/

class CreateNewBackupReactor : public ClientBidiReactorBase<backup::CreateNewBackupRequest, backup::CreateNewBackupResponse>
{
  enum class State {
    KEY_ENTROPY = 1,
    DATA_HASH = 2,
    CHUNKS = 3,
  };
  const size_t chunkLimit;
  size_t currentChunk = 0;
  State state = State::KEY_ENTROPY;
public:
  CreateNewBackupReactor(size_t chunkLimit) : chunkLimit(chunkLimit) {
    std::cout << "create new backup init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::CreateNewBackupRequest &request, std::shared_ptr<backup::CreateNewBackupResponse> previousResponse) override
  {
    std::cout << "here prepare request" << std::endl;
    // send key entropy
    if (this->state == State::KEY_ENTROPY) {
      std::string keyEntropy = randomString();
      std::cout << "here prepare request key entropy: " << keyEntropy << std::endl;
      request.set_keyentropy(keyEntropy);
      this->state = State::DATA_HASH;
      return nullptr;
    }
    // send data hash
    if (this->state == State::DATA_HASH) {
      std::string hash = randomString();
      std::cout << "here prepare request data hash " << hash << std::endl;
      request.set_newcompactionhash(hash);
      this->state = State::CHUNKS;
      return nullptr;
    }
    // send chunks
    if (this->currentChunk >= this->chunkLimit) {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string dataChunk = randomString(1000);
    std::cout << "here prepare request data chunk " << this->currentChunk << "/" << dataChunk.size() << std::endl;
    request.set_newcompactionchunk(dataChunk);
    ++this->currentChunk;
    return nullptr;
  }
};

class SendLogReactor : public ClientWriteReactorBase<backup::SendLogRequest, google::protobuf::Empty> {
  const size_t chunkLimit;
  size_t currentChunk = 0;

public:
  SendLogReactor(size_t chunkLimit) : chunkLimit(chunkLimit) {
    std::cout << "send log init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::SendLogRequest &request) override {
    if (this->currentChunk >= this->chunkLimit) {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string dataChunk = randomString(1000);
    std::cout << "here prepare request data chunk " << this->currentChunk << "/" << dataChunk.size() << std::endl;
    request.set_logdata(dataChunk);
    ++this->currentChunk;
    return nullptr;
  }
};

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;

public:
  Client(std::shared_ptr<grpc::Channel> channel);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;
  std::unique_ptr<SendLogReactor> sendLogReactor;

  void createNewBackup();
  void sendLog();
  void recoverBackupKey();
  void pullBackup();

  bool reactorActive();
};

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
#include <thread>

/*
SEND_USER_ID
client => userID => server
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
  enum class State
  {
    USER_ID = 1,
    KEY_ENTROPY = 2,
    DATA_HASH = 3,
    CHUNKS = 4,
  };
  const std::string userID;
  const size_t chunkLimit;
  size_t currentChunk = 0;
  State state = State::USER_ID;
  std::string backupID;
  const std::function<void(const std::string&)> setLasBackupIDCallback;
public:
  CreateNewBackupReactor(const std::string &userID, size_t chunkLimit, const std::function<void(const std::string &)> &setLasBackupIDCallback) : userID(userID), chunkLimit(chunkLimit), setLasBackupIDCallback(setLasBackupIDCallback)
  {
    std::cout << "create new backup init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::CreateNewBackupRequest &request, std::shared_ptr<backup::CreateNewBackupResponse> previousResponse) override
  {
    std::cout << "here prepare request [" << std::hash<std::thread::id>{}(std::this_thread::get_id())
              << "]" << std::endl;
    if (!previousResponse->backupid().empty()) {
      this->backupID = previousResponse->backupid();
      std::cout << "got backup id: " << this->backupID << std::endl;
    }
    // send user id
    if (this->state == State::USER_ID) {
      std::cout << "here prepare request user id: " << this->userID << std::endl;
      request.set_userid(this->userID);
      this->state = State::KEY_ENTROPY;
      return nullptr;
    }
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

  void doneCallback() override {
    std::cout << "create new backup DONE, backup id: " << this->backupID << std::endl;
    this->setLasBackupIDCallback(this->backupID);
  } 
};

class SendLogReactor : public ClientWriteReactorBase<backup::SendLogRequest, google::protobuf::Empty>
{
  enum class State {
    USER_ID = 1,
    BACKUP_ID = 2,
    LOG_HASH = 3,
    LOG_CHUNK = 4,
  };

  State state = State::USER_ID;
  const size_t chunkLimit;
  size_t currentChunk = 0;
  const std::string userID;
  std::string lastBackupID;
public:
  SendLogReactor(const std::string &userID, size_t chunkLimit, std::string lastBackupID) : userID(userID), chunkLimit(chunkLimit), lastBackupID(lastBackupID)
  {
    if (this->userID.empty()) {
      throw std::runtime_error("user id cannot be empty");
    }
    std::cout << "send log init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::SendLogRequest &request) override {
    if (this->state == State::USER_ID) {
      request.set_userid(this->userID);
      this->state = State::BACKUP_ID;
      return nullptr;
    }
    if (this->state == State::BACKUP_ID) {
      request.set_backupid(this->lastBackupID);
      this->state = State::LOG_HASH;
      return nullptr;
    }
    if (this->state == State::LOG_HASH) {
      // todo calculate hash
      request.set_loghash(randomString());
      this->state = State::LOG_CHUNK;
      return nullptr;
    }
    if (this->state == State::LOG_CHUNK) {
      size_t size = randomNumber(0, 1) ? 1024 * 1024 * 2 : 1024;
      if (this->currentChunk >= this->chunkLimit) {
        return std::make_unique<grpc::Status>(grpc::Status::OK);
      }
      std::string dataChunk = mockBytes(size);
      std::cout << "here prepare request data chunk " << this->currentChunk << "/" << dataChunk.size() << std::endl;
      request.set_logdata(dataChunk);
      ++this->currentChunk;
      return nullptr;
    }
    throw std::runtime_error("invalid state");
  }

  void doneCallback() override {
    std::cout << "send log done: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
  }
};

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;
  const std::string userID;
  std::string lastBackupID;

  const std::function<void(const std::string&)> setLasBackupIDCallback = [this](const std::string &backupID){
    this->lastBackupID = backupID;
  };

public:
  Client(std::shared_ptr<grpc::Channel> channel, const std::string &userID);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;
  std::unique_ptr<SendLogReactor> sendLogReactor;

  void createNewBackup();
  void sendLog();
  void recoverBackupKey();
  void pullBackup();

  bool reactorActive();

  std::string getCurrentBackupID () {
    return this->lastBackupID;
  }
};

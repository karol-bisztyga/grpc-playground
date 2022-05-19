#include "ClientWriteReactorBase.h"

#include "Tools.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>

class AddAttachmentReactor : public ClientWriteReactorBase<backup::AddAttachmentRequest, google::protobuf::Empty>
{
  enum class State
  {
    USER_ID = 1,
    BACKUP_ID = 2,
    LOG_ID = 3,
    DATA_HASH = 4,
    DATA_CHUNK = 5,
  };

  State state = State::USER_ID;
  const size_t chunkLimit;
  size_t currentChunk = 0;
  const std::string userID;
  std::string backupID;
  std::string logID;
  std::string hash;

public:
  AddAttachmentReactor(const std::string &userID, std::string backupID, std::string logID, size_t chunkLimit) : userID(userID), backupID(backupID), logID(logID), chunkLimit(chunkLimit)
  {
    if (this->userID.empty())
    {
      throw std::runtime_error("user id cannot be empty");
    }
    if (this->backupID.empty())
    {
      throw std::runtime_error("backup id cannot be empty");
    }
    std::cout << "send log init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::AddAttachmentRequest &request) override
  {
    if (this->state == State::USER_ID)
    {
      request.set_userid(this->userID);
      this->state = State::BACKUP_ID;
      return nullptr;
    }
    if (this->state == State::BACKUP_ID)
    {
      request.set_backupid(this->backupID);
      this->state = State::LOG_ID;
      return nullptr;
    }
    if (this->state == State::LOG_ID)
    {
      if (!this->logID.empty())
      {
        request.set_logid(this->logID);
      }
      this->state = State::DATA_HASH;
      return nullptr;
    }
    if (this->state == State::DATA_HASH)
    {
      // todo calculate hash
      request.set_datahash(randomString());
      this->state = State::DATA_CHUNK;
      return nullptr;
    }
    if (this->state == State::DATA_CHUNK)
    {
      size_t size = randomNumber(0, 1) ? 1024 * 1024 * 2 : 1024;
      if (this->currentChunk >= this->chunkLimit) {
        return std::make_unique<grpc::Status>(grpc::Status::OK);
      }
      std::string dataChunk = mockBytes(size);
      request.set_datachunk(dataChunk);
      ++this->currentChunk;
      return nullptr;
    }
    throw std::runtime_error("invalid state");
  }

  void doneCallback() override {
    std::cout << "send attachment done: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
    if (this->status.ok()) {
      std::cout << "send attachment successful" << std::endl;
    }
  }
};

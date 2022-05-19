#include "ClientWriteReactorBase.h"

#include "Tools.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>

class SendLogReactor : public ClientWriteReactorBase<backup::SendLogRequest, backup::SendLogResponse>
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

  const std::function<void(const std::string &)> setLasBackupIDCallback;

public:
  SendLogReactor(const std::string &userID, size_t chunkLimit, std::string lastBackupID, const std::function<void(const std::string &)> &setLasBackupIDCallback) : userID(userID), chunkLimit(chunkLimit), lastBackupID(lastBackupID), setLasBackupIDCallback(setLasBackupIDCallback)
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
      request.set_logdata(dataChunk);
      ++this->currentChunk;
      return nullptr;
    }
    throw std::runtime_error("invalid state");
  }

  void doneCallback() override {
    std::cout << "send log done: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
    if (this->status.ok()) {
      std::cout << "send log successful - new log id is: " << this->response.logid() << std::endl;
      this->setLasBackupIDCallback(this->response.logid());
    }
  }
};

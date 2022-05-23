#include "ClientBidiReactorBase.h"

#include <string>
#include <memory>

/*
SEND_USER_ID
client => userID => server
SEND_DEVICE_ID
client => deviceID => server
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
    DEVICE_ID = 2,
    KEY_ENTROPY = 3,
    DATA_HASH = 4,
    CHUNKS = 5,
  };
  const std::string userID;
  const std::string deviceID;
  const size_t chunkLimit;
  size_t currentChunk = 0;
  State state = State::USER_ID;
  std::string backupID;
  const std::function<void(const std::string &)> setLastBackupIDCallback;

public:
  CreateNewBackupReactor(const std::string &userID, const std::string &deviceID, size_t chunkLimit, const std::function<void(const std::string &)> &setLastBackupIDCallback) : userID(userID), deviceID(deviceID), chunkLimit(chunkLimit), setLastBackupIDCallback(setLastBackupIDCallback)
  {
    std::cout << "create new backup init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::CreateNewBackupRequest &request, std::shared_ptr<backup::CreateNewBackupResponse> previousResponse) override
  {
    if (!previousResponse->backupid().empty())
    {
      this->backupID = previousResponse->backupid();
      std::cout << "got backup id: " << this->backupID << std::endl;
    }
    // send user id
    if (this->state == State::USER_ID)
    {
      request.set_userid(this->userID);
      this->state = State::DEVICE_ID;
      return nullptr;
    }
    // send device id
    if (this->state == State::DEVICE_ID)
    {
      request.set_deviceid(this->deviceID);
      this->state = State::KEY_ENTROPY;
      return nullptr;
    }
    // send key entropy
    if (this->state == State::KEY_ENTROPY)
    {
      std::string keyEntropy = randomString();
      request.set_keyentropy(keyEntropy);
      this->state = State::DATA_HASH;
      return nullptr;
    }
    // send data hash
    if (this->state == State::DATA_HASH)
    {
      std::string hash = randomString();
      request.set_newcompactionhash(hash);
      this->state = State::CHUNKS;
      return nullptr;
    }
    // send chunks
    if (this->currentChunk >= this->chunkLimit)
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string dataChunk = randomString(1000);
    request.set_newcompactionchunk(dataChunk);
    ++this->currentChunk;
    return nullptr;
  }

  void doneCallback() override
  {
    if (!this->status.ok()) {
      return;
    }
    std::cout << "create new backup DONE, backup id: " << this->backupID << std::endl;
    this->setLastBackupIDCallback(this->backupID);
  }
};

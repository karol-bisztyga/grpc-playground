#include "ClientReadReactorBase.h"

#include "Tools.h"

#include <string>
#include <memory>
#include <thread>

class PullBackupReactor : public ClientReadReactorBase<backup::PullBackupRequest, backup::PullBackupResponse>
{
public:
  std::unique_ptr<grpc::Status> readResponse(const backup::PullBackupResponse &response) override {
    std::cout << "read response" << std::endl;
    if (response.has_compactionchunk()) {
      std::cout << "has compaction chunk " << response.compactionchunk().size() << std::endl;
    }
    if (response.has_logchunk()) {
      std::cout << "has log chunk " << response.logchunk().size() << std::endl;
    }
    return nullptr;
  }

  void doneCallback() override {
    std::cout << "pull backup done: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
  }
};

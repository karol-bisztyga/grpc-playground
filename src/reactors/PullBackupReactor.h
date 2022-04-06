#include "ClientReadReactorBase.h"

#include "Tools.h"

#include <string>
#include <memory>
#include <thread>

class PullBackupReactor : public ClientReadReactorBase<backup::PullBackupRequest, backup::PullBackupResponse>
{
public:
  std::unique_ptr<grpc::Status> readResponse(const backup::PullBackupResponse &response) override {
    std::cout << "read response " << response.has_compactionchunk() << "/" << response.has_logchunk() << std::endl;
    return nullptr;
  }

  void doneCallback() override {
    std::cout << "pull backup done: " << this->status.error_code() << "/" << this->status.error_message() << std::endl;
  }
};

#include "Client.h"

Client::Client(std::shared_ptr<grpc::Channel> channel, std::string id)
    : stub(backup::BackupService::NewStub(channel)),
      id(id) {}

void Client::sendLog(const std::string data)
{
  grpc::ClientContext context;
  backup::SendLogRequest request;
  backup::SendLogResponse response;

  request.set_id(this->id);
  request.set_data(data);

  grpc::Status status = this->stub->SendLog(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
  std::cout << "log sent" << std::endl;
}

void Client::resetLog()
{
  grpc::ClientContext context;
  backup::ResetLogsRequest request;
  backup::ResetLogsResponse response;

  request.set_id(this->id);

  grpc::Status status = this->stub->ResetLogs(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
  std::cout << "log reset" << std::endl;
}

void Client::restoreBackup()
{
  grpc::ClientContext context;
  backup::RestoreRequest request;

  request.set_id(this->id);

  std::unique_ptr<grpc::ClientReader<backup::RestoreResponse> > stream = this->stub->Restore(&context, request);
  backup::RestoreResponse response;
  std::cout << "reading the restore stream:" << std::endl;
  while (stream->Read(&response))
  {
    std::cout << "received: [" << response.data() << "]" << std::endl;
  }
  std::cout << "done reading the restore stream" << std::endl;
}

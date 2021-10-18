#include "Client.h"

Client::Client(std::shared_ptr<grpc::Channel> channel, std::string id)
    : stub(backup::BackupService::NewStub(channel)),
      id(id) {}

void Client::resetKey(const std::string newKey, const std::vector<std::string> newCompact)
{
  grpc::ClientContext context;
  backup::ResetKeyResponse response;

  std::unique_ptr<grpc::ClientWriter<backup::ResetKeyRequest>> writer = this->stub->ResetKey(&context, &response);

  backup::ResetKeyRequest request;
  request.set_userid(this->id);
  request.set_newkey(newKey);

  for (std::string chunk : newCompact)
  {
    std::cout << "trying to write [" << chunk << "]" << std::endl;
    request.set_newcompactchunk(chunk);
    if (!writer->Write(request))
    {
      std::cout << "stream interrupted" << std::endl;
      return;
    }
  }
  writer->WritesDone();
  grpc::Status status = writer->Finish();
  if (!status.ok())
  {
    throw std::runtime_error("writer error");
  }
  std::cout << "done writing chunks" << std::endl;
  }

void Client::sendLog(const std::string data)
{
  grpc::ClientContext context;
  backup::SendLogRequest request;
  backup::SendLogResponse response;

  request.set_userid(this->id);
  request.set_data(data);

  std::cout << "sending log [" << data << "]" << std::endl;

  grpc::Status status = this->stub->SendLog(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
  std::cout << "log sent" << std::endl;
}

void Client::pullBackupKey(const std::string pakeKey)
{
  grpc::ClientContext context;
  backup::PullBackupKeyRequest request;
  backup::PullBackupKeyResponse response;

  request.set_userid(this->id);
  request.set_pakekey(pakeKey);

  grpc::Status status = this->stub->PullBackupKey(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }

  std::string backupKey = response.backupkey();

  std::cout << "pull backup key, received [" << backupKey << "]" << std::endl;
}

void Client::pullCompact()
{
  grpc::ClientContext context;
  backup::PullCompactRequest request;
  backup::PullCompactResponse response;

  request.set_userid(this->id);

  std::unique_ptr<grpc::ClientReader<backup::PullCompactResponse>> stream = this->stub->PullCompact(&context, request);
  std::cout << "reading compact stream:" << std::endl;
  while (stream->Read(&response))
  {
    std::string compactChunk = response.compactchunk();
    std::string logChunk = response.logchunk();
    if (compactChunk.size())
    {
      std::cout << "received: [" << compactChunk << "]" << std::endl;
    }
    if (logChunk.size())
    {
      std::cout << "received: [" << logChunk << "]" << std::endl;
    }
  }
  std::cout << "done reading the restore stream" << std::endl;
}

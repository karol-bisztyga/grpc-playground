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

  // first send a key and an empty chunk
  request.set_newkey(newKey);
    std::cout << "trying to write a new key [" << newKey << "]" << std::endl;
  if (!writer->Write(request))
  {
    std::cout << "stream interrupted" << std::endl;
    return;
  }

  // then send an empty key and filled chunks
  request.set_newkey("");
  for (std::string chunk : newCompact)
  {
    std::cout << "trying to write chunk [" << chunk << "]" << std::endl;
    request.set_compactionchunk(chunk);
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
  if (!status.ok() || !response.success())
  {
    throw std::runtime_error(status.error_message());
  }

  std::string backupKey = response.encryptedbackupkey();

  std::cout << "pull backup key, received [" << backupKey << "]" << std::endl;
}

void Client::pullCompact()
{
  grpc::ClientContext context;
  backup::PullCompactionRequest request;
  backup::PullCompactionResponse response;

  request.set_userid(this->id);

  std::unique_ptr<grpc::ClientReader<backup::PullCompactionResponse>> stream = this->stub->PullCompaction(&context, request);
  std::cout << "reading compact stream:" << std::endl;
  while (stream->Read(&response))
  {
    std::string compactionChunk = response.compactionchunk();
    std::string logChunk = response.logchunk();
    if (compactionChunk.size())
    {
      std::cout << "received[c]: [" << compactionChunk << "]" << std::endl;
    }
    if (logChunk.size())
    {
      std::cout << "received[l]: [" << logChunk << "]" << std::endl;
    }
  }
  std::cout << "done reading the restore stream" << std::endl;
}

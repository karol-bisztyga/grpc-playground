#include "Client.h"

Client::Client(std::shared_ptr<grpc::Channel> channel)
    : stub(blob::BlobService::NewStub(channel)) {}


void Client::put(const std::string &reverseIndex, std::function<std::string()> &dataChunksObtainer) {
  grpc::ClientContext context;
  blob::PutRequest request;
  ::google::protobuf::Empty response;
  std::unique_ptr<grpc::ClientWriter<blob::PutRequest>> writer = this->stub->Put(&context, &response);

  request.set_reverseindex(reverseIndex);
  if (!writer->Write(request))
  {
    std::cout << "stream interrupted" << std::endl;
    return;
  }
  request.set_reverseindex("");

  size_t chunkCounter = 0;
  do {
    std::cout << "reading chunk " << ++chunkCounter << std::endl;
    request.set_datachunk(dataChunksObtainer());
    if (!writer->Write(request))
    {
      std::cout << "stream interrupted" << std::endl;
      return;
    }
  } while(request.datachunk().size());

  writer->WritesDone();
  grpc::Status status = writer->Finish();
  if (!status.ok())
  {
    throw std::runtime_error("writer error");
  }
  std::cout << "done writing chunks" << std::endl;
}

void Client::get(const std::string &reverseIndex, std::function<void(std::string)> &callback) {
  grpc::ClientContext context;
  blob::GetRequest request;
  blob::GetResponse response;

  request.set_reverseindex(reverseIndex);

  std::unique_ptr<grpc::ClientReader<blob::GetResponse>> stream = this->stub->Get(&context, request);
  std::cout << "reading stream:" << std::endl;

  while (stream->Read(&response))
  {
    std::string dataChunk = response.datachunk();
    std::cout << "received: [" << dataChunk << "]" << std::endl;
    callback(dataChunk);
  }
  std::cout << "done reading the restore stream" << std::endl;
}

bool Client::remove(const std::string &reverseIndex) {
  ;
}

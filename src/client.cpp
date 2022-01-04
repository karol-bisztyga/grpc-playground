#include "Client.h"
#include "Tools.h"

Client::Client(std::shared_ptr<grpc::Channel> channel)
    : stub(blob::BlobService::NewStub(channel)) {}

bool Client::put(const std::string &reverseIndex, const std::string &hash, const std::string &data)
{
  grpc::ClientContext context;
  blob::PutRequest request;
  blob::PutResponse response;
  std::unique_ptr<grpc::ClientReaderWriter<blob::PutRequest, blob::PutResponse> > stream = this->stub->Put(&context);

  request.set_holder(reverseIndex);
  if (!stream->Write(request)) {
    std::cout << "stream interrupted 1" << std::endl;
    return false;
  }
  request.set_holder("");
  request.set_blobhash(hash);
  if (!stream->Write(request)) {
    std::cout << "stream interrupted 2" << std::endl;
    return false;
  }

  if(!stream->Read(&response)) {
    throw std::runtime_error("reading put response failed");
  }
  if (!response.dataexists()) {
    std::cout << "data does not exist, uploading..." << std::endl;
    request.set_blobhash("");
    const size_t chunkSize = GRPC_CHUNK_SIZE_LIMIT - GRPC_METADATA_SIZE_PER_MESSAGE;
    for (size_t i = 0; i < data.size();)
    {
      const size_t len = std::min(i + chunkSize, data.size()) - i;
      std::cout << "writing chunk " << i << "-" << (len + i) << std::endl;
      request.set_datachunk(data.substr(i, len));
      if (!stream->Write(request))
      {
        std::cout << "failed to write parts, aborting on bytes: " << std::endl;
        // in a case when the item already exists on S3 we just want to gracefully
        // abort pushing parts but it still can be a successfull operation
        break;
      }
      i += chunkSize;
    }
  } else {
    std::cout << "data exists, aborted upload" << std::endl;
  }
  std::cout << "finishing" << std::endl;

  stream->WritesDone();
  grpc::Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << "an error ocurred: " << status.error_message() << std::endl;
    return false;
  }
  std::cout << "done writing chunks" << std::endl;
  return true;
}

void Client::get(const std::string &reverseIndex, std::function<void(std::string)> &callback) {
  grpc::ClientContext context;
  blob::GetRequest request;
  blob::GetResponse response;

  request.set_holder(reverseIndex);

  std::unique_ptr<grpc::ClientReader<blob::GetResponse>> stream = this->stub->Get(&context, request);
  std::cout << "reading stream:" << std::endl;

  while (stream->Read(&response))
  {
    std::string dataChunk = response.datachunk();
    std::cout << "received: [" << dataChunk << "]" << std::endl;
    callback(dataChunk);
  }
  grpc::Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << "an error ocurred: " << status.error_message() <<  std::endl;
    return;
  }
  std::cout << "done reading the restore stream" << std::endl;
}

bool Client::remove(const std::string &reverseIndex) {
  grpc::ClientContext context;
  blob::RemoveRequest request;
  google::protobuf::Empty response;

  request.set_holder(reverseIndex);

  grpc::Status status = this->stub->Remove(&context, request, &response);
  if (!status.ok()) {
    std::cout << "an error ocurred: " << status.error_message() << std::endl;
    return false;
  }
  std::cout << "done removing " << reverseIndex << std::endl;
  return true;
}

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include <grpcpp/grpcpp.h>

#include "../_generated/blob.pb.h"
#include "../_generated/blob.grpc.pb.h"

#include "ClientBidiReactorBase.h"
#include "ClientReadReactorBase.h"
#include "Tools.h"

enum class PutState {
  SEND_HOLDER = 0,
  SEND_HASH = 1,
  SEND_CHUNKS = 2,
};

class PutReactor : public ClientBidiReactorBase<blob::PutRequest, blob::PutResponse>
{
  PutState state = PutState::SEND_HOLDER;
  const std::string hash;
  const std::string reverseIndex;
  const std::string data;
  size_t currentDataSize = 0;
  const size_t chunkSize = GRPC_CHUNK_SIZE_LIMIT - GRPC_METADATA_SIZE_PER_MESSAGE;
  std::unordered_map<std::string, std::string> *persist;
public:
  PutReactor(const std::string &reverseIndex, const std::string &hash, const std::string &data, std::unordered_map<std::string, std::string> *persist) : reverseIndex(reverseIndex), hash(hash), data(data), persist(persist) {}

  std::unique_ptr<grpc::Status> prepareRequest(blob::PutRequest &request, std::shared_ptr<blob::PutResponse> previousResponse) override
  {
    if (this->state == PutState::SEND_HOLDER)
    {
      this->request.set_holder(this->reverseIndex);
      this->state = PutState::SEND_HASH;
      return nullptr;
    }
    if (this->state == PutState::SEND_HASH)
    {
      request.set_holder("");
      request.set_blobhash(this->hash);
      this->state = PutState::SEND_CHUNKS;
      return nullptr;
    }
    request.set_blobhash("");
    if (previousResponse->dataexists()) {
      std::cout << "data exists - aborting" << std::endl;
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::cout << "data NOT exists - continue [" << this->currentDataSize << "][" << this->data.size() << "]" << std::endl;
    if (this->currentDataSize >= this->data.size())
    {
      std::cout << "wrote all data" << std::endl;
      this->persist->insert(std::pair<std::string, std::string>(this->reverseIndex, this->hash));
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    const size_t nextChunkSize = std::min(this->chunkSize, this->data.size() - this->currentDataSize);
    std::string currentData = this->data.substr(this->currentDataSize, nextChunkSize);
    std::cout << "writing chunk " << this->currentDataSize << "-" << (this->currentDataSize + nextChunkSize) << " out of " << this->data.size() << " total" <<  std::endl;
    request.set_datachunk(currentData);
    this->currentDataSize += nextChunkSize;
    return nullptr;
  }
};

class GetReactor : public ClientReadReactorBase<blob::GetRequest, blob::GetResponse>
{
public:
  std::unique_ptr<grpc::Status> readResponse(const blob::GetResponse &response) override
  {
    std::cout << "received: [" << response.datachunk() << "]" << std::endl;
    return nullptr;
  }

  void doneCallback() override {
    std::cout << "done receiving parts" << std::endl;
  }
};

class RemoveReactor : public grpc::ClientUnaryReactor
{
  std::unordered_map<std::string, std::string> *persist;
  bool done = false;

public:
  grpc::ClientContext context;
  blob::RemoveRequest request;
  google::protobuf::Empty response;

  RemoveReactor(std::unordered_map<std::string, std::string> *persist) : persist(persist) {}

  void OnDone(const grpc::Status &status) override
  {
    std::cout << "removing done for " << this->request.holder() << std::endl;
    this->persist->erase(this->request.holder());
    this->done = true;
  }

  bool isDone() {
    return this->done;
  }
};

class Client
{
  std::unique_ptr<blob::BlobService::Stub> stub;

public:
  // this represents what's have to be persisted by the users of the blob service
  // note, that hashes will not have to be persisted
  // they're going to be calculated on the fly but we're going to keep them here
  // to make those tests simpler for us
  //               reverse index, hash
  std::unordered_map<std::string, std::string> persist; 

  Client(std::shared_ptr<grpc::Channel> channel);

  std::unique_ptr<PutReactor> putReactor;
  std::unique_ptr<GetReactor> getReactor;
  std::unique_ptr<RemoveReactor> removeReactor;

  void put(const std::string &reverseIndex, const std::string &hash, const std::string &data);
  void get(const std::string &reverseIndex);
  void remove(const std::string &reverseIndex);

  bool reactorActive();
};

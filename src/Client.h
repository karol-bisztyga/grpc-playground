#pragma once

#include <grpcpp/grpcpp.h>

#include "../_generated/blob.pb.h"
#include "../_generated/blob.grpc.pb.h"

#include <string>
#include <memory>
#include <unordered_map>

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

  bool put(const std::string &reverseIndex, const std::string &hash, const std::string &data);
  void get(const std::string &reverseIndex, std::function<void(std::string)> &callback);
  bool remove(const std::string &reverseIndex);
};

#pragma once

#include <grpcpp/grpcpp.h>

#include "../_generated/blob.pb.h"
#include "../_generated/blob.grpc.pb.h"

#include <string>
#include <memory>

class Client
{
  std::unique_ptr<blob::BlobService::Stub> stub;
public:
  Client(std::shared_ptr<grpc::Channel> channel);

  void put(const std::string &reverseIndex, std::function<std::string()> &dataChunksObtainer);
  void get(const std::string &reverseIndex, std::function<void(std::string)> &callback);
  bool remove(const std::string &reverseIndex);
};

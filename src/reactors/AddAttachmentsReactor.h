#pragma once

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <string>
#include <memory>

class AddAttachmentsReactor : public grpc::ClientUnaryReactor
{
public:
  grpc::ClientContext context;
  backup::AddAttachmentsRequest request;
  google::protobuf::Empty response;

  void OnDone(const grpc::Status &status) override
  {
    std::cout << "adding attachments done" << std::endl;
  }
};

#pragma once

#include "../_generated/inner.grpc.pb.h"
#include "../_generated/inner.pb.h"

#include <grpcpp/grpcpp.h>

#include <string>

class InnerServiceImpl final : public inner::InnerService::CallbackService {
public:
  InnerServiceImpl() {}
  virtual ~InnerServiceImpl() {}

  grpc::ServerBidiReactor<
      inner::TalkBetweenServicesRequest,
      inner::TalkBetweenServicesResponse> *
  TalkBetweenServices(grpc::CallbackServerContext *context) override;
};

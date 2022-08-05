#pragma once

#include "TalkWithClientReactor.h"

#include "../_generated/outer.grpc.pb.h"
#include "../_generated/outer.pb.h"

#include <grpcpp/grpcpp.h>

class OuterServiceImpl final : public outer::OuterService::CallbackService {

public:
  grpc::ServerBidiReactor<
      outer::TalkWithClientRequest,
      outer::TalkWithClientResponse> *
  TalkWithClient(grpc::CallbackServerContext *context) override;
};

#include "InnerServiceImpl.h"

#include "TalkReactor.h"

#include <memory>

grpc::ServerBidiReactor<
    inner::TalkBetweenServicesRequest,
    inner::TalkBetweenServicesResponse> *
InnerServiceImpl::TalkBetweenServices(grpc::CallbackServerContext *context) {
  return new TalkReactor();
}

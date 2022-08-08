#include "OuterServiceImpl.h"

grpc::ServerBidiReactor<
    outer::TalkWithClientRequest,
    outer::TalkWithClientResponse> *
OuterServiceImpl::TalkWithClient(grpc::CallbackServerContext *context) {
  return new TalkWithClientReactor(this->reactorsQueue);
}

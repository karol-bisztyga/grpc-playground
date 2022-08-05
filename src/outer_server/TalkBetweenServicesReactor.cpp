#include "TalkBetweenServicesReactor.h"

#include <iostream>
#include <thread>

void TalkBetweenServicesReactor::scheduleMessage(
    std::unique_ptr<std::string> msg) {
  const size_t size = msg->size();
  this->messages.enqueue(std::move(*msg));
}

std::unique_ptr<grpc::Status> TalkBetweenServicesReactor::prepareRequest(
    inner::TalkBetweenServicesRequest &request,
    std::shared_ptr<inner::TalkBetweenServicesResponse> previousResponse) {
  std::string msg;
  msg = this->messages.dequeue();
  // flow is lost after this place
  if (msg.empty()) {
    return std::make_unique<grpc::Status>(grpc::Status::OK);
  }
  request.set_msg(msg);
  return nullptr;
}

void TalkBetweenServicesReactor::doneCallback() {
  this->terminationNotifier->notify_one();
}

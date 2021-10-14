#include "TunnelBrokerClient.h"

TunnelBrokerClient::TunnelBrokerClient(std::shared_ptr<grpc::Channel> channel, std::string id, std::string deviceToken)
    : stub_(tunnelbroker::TunnelBrokerService::NewStub(channel)),
      id(id),
      deviceToken(deviceToken) {}

tunnelbroker::CheckResponseType TunnelBrokerClient::checkIfPrimaryDeviceOnline()
{
  grpc::ClientContext context;
  tunnelbroker::CheckRequest request;
  tunnelbroker::CheckResponse response;

  request.set_userid(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = stub_->CheckIfPrimaryDeviceOnline(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
  return response.checkresponsetype();
}

bool TunnelBrokerClient::becomeNewPrimaryDevice()
{
  grpc::ClientContext context;
  tunnelbroker::NewPrimaryRequest request;
  tunnelbroker::NewPrimaryResponse response;

  request.set_userid(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = stub_->BecomeNewPrimaryDevice(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
  return response.success();
}

void TunnelBrokerClient::sendPong()
{
  grpc::ClientContext context;
  tunnelbroker::PongRequest request;
  tunnelbroker::PongResponse response;

  request.set_userid(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = stub_->SendPong(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
}
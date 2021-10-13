#include "TunnelBrokerClient.h"

TunnelBrokerClient::TunnelBrokerClient(std::shared_ptr<grpc::Channel> channel, std::string id, std::string deviceToken)
    : tbStub_(tunnelbroker::TunnelBrokerService::NewStub(channel)),
      backupStub_(tunnelbroker::BackupService::NewStub(channel)),
      id(id),
      deviceToken(deviceToken) {}

tunnelbroker::CheckResponseType TunnelBrokerClient::checkIfPrimaryDeviceOnline()
{
  grpc::ClientContext context;
  tunnelbroker::CheckRequest request;
  tunnelbroker::CheckResponse response;

  request.set_id(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = tbStub_->CheckIfPrimaryDeviceOnline(&context, request, &response);
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

  request.set_id(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = tbStub_->BecomeNewPrimaryDevice(&context, request, &response);
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

  request.set_id(this->id);
  request.set_devicetoken(this->deviceToken);

  grpc::Status status = tbStub_->SendPong(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
}

void TunnelBrokerClient::sendLog(const std::string data)
{
  grpc::ClientContext context;
  tunnelbroker::SendLogRequest request;
  tunnelbroker::SendLogResponse response;

  request.set_id(this->id);
  request.set_data(data);

  grpc::Status status = backupStub_->SendLog(&context, request, &response);
  if (!status.ok())
  {
    throw std::runtime_error(status.error_message());
  }
}

void TunnelBrokerClient::resetLog() {
  ;
}

void TunnelBrokerClient::restoreBackup() {
  ;
}

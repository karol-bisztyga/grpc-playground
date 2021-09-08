#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::Status;
using grpc::ClientWriter;

using ping::HostPingResponse;
using ping::HostPingRequest;
using ping::InitialRequest;
using ping::InitialResponse;
using ping::InitialResponseType;
using ping::PingService;
using ping::SendPingRequest;
using ping::SendPingResponse;
using ping::SendPingResponseType;

class PingClient
{
public:
  PingClient(std::shared_ptr<Channel> channel, size_t id)
      : stub_(PingService::NewStub(channel)), id(id) {}

  ping::InitialResponseType Initialize()
  {
    ClientContext context;
    InitialRequest request;
    InitialResponse response;
    InitialResponseType responseType;
    request.set_id(this->id);
    Status status = stub_->Initialize(&context, request, &response);
    if (!status.ok())
    {
      throw std::runtime_error(status.error_message());
    }

    responseType = response.initialresponsetype();
    return responseType;
  }

  SendPingResponseType SendPing()
  {
    ClientContext context;
    SendPingRequest request;
    SendPingResponse response;
    request.set_id(this->id);
    Status status = stub_->SendPing(&context, request, &response);
    if (!status.ok())
    {
      throw std::runtime_error(status.error_message());
    }
    SendPingResponseType responseType = response.sendpingresponsetype();
    return responseType;
  }

  void HostPing()
  {
    ClientContext context;
    std::unique_ptr<ClientReaderWriter<HostPingRequest, HostPingResponse> > stream = stub_->HostPing(&context);

    HostPingRequest request;
    HostPingResponse response;

    request.set_id(this->id);
    stream->Write(request);

    while(stream->Read(&response))
    {
      std::cout << "client sending a PONG" << std::endl;
      stream->Write(request);
    }
  }

private:
  std::unique_ptr<PingService::Stub> stub_;
  const size_t id;
};

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "please provide an argument with a client id" << std::endl;
    return 1;
  }
  size_t id = (size_t)atoi(argv[1]);
  std::cout << "client start, id: " << id << std::endl;
  std::string target_str = "localhost:50051";

  PingClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()), id);
  InitialResponseType initialResponse = client.Initialize();
  std::cout << "initial response: " << ping::InitialResponseType_Name(initialResponse) << std::endl;

  if (initialResponse == InitialResponseType::NEW_PRIMARY)
  {
    client.HostPing();
  }
  else
  {
    SendPingResponseType responseType = client.SendPing();
    if (responseType == SendPingResponseType::PRIMARY_ONLINE)
    {
      std:: cout << "primary device is online" << std::endl;
      // TODO some actions here
    }
    else
    {
      std::cout << "primary device is offline" << std::endl;
      // TODO: the question here is: should we terminate the client or leave it
      // so it becomes a primary device once the original one perishes?
      // to do that we'd probably need an additional queue of primary candidates
      // and a non busy loop here(also some additional fields in ResponseType)
      // for now it terminates
    }
  }

  return 0;
}

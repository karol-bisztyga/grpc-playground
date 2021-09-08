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
      std::cout << "ERROR: " << std::to_string(status.error_code()) << ": " << status.error_message() << std::endl; // todo remove
    }

    responseType = response.initialresponsetype();
    return responseType;
  }

  void SendPing()
  {
    ClientContext context;
    SendPingRequest request;
    SendPingResponse response;
    request.set_id(this->id);
    Status status = stub_->SendPing(&context, request, &response);
    SendPingResponseType responseType = response.sendpingresponsetype();

    std::cout << "send ping response: " << ping::SendPingResponseType_Name(responseType) << std::endl;
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

  /*
    ClientContext context;
    std::unique_ptr<ClientReaderWriter<PingRequest, PingResponse> > stream = stub_->Ping(&context);
    PingRequest request;
    PingResponse response;
    request.set_requesttype(RequestType::INITIAL);
    request.set_id(this->id);
    stream->Write(request);
    ResponseType responseType = ResponseType::WAIT;
    while (stream->Read(&response))
    {
      responseType = response.responsetype();
      std::cout << "here client ping #read " << ping::ResponseType_Name(responseType) << std::endl;
      if (responseType != ResponseType::WAIT)
      {
        break;
      }
    }

    do
    {
      responseType = response.responsetype();
      std::cout << "new data from server: " << ping::ResponseType_Name(responseType) << std::endl;
      if (responseType == ResponseType::PING)
      {
        std::cout << "sending PONG" << std::endl;
        request.set_requesttype(RequestType::PONG);
        stream->Write(request);
      }
    } while (stream->Read(&response));

    Status status = this->stream->Finish();
    if (!status.ok())
    {
      std::cout << "ERROR: " << std::to_string(status.error_code()) << ": " << status.error_message() << std::endl;
    }
    std::cout << "client with id " << this->id << " disconnected successfully" << std::endl;
  }
    */

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
    client.SendPing();
  }

  return 0;
}

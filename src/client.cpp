#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::Status;

using ping::PingRequest;
using ping::PingResponse;
using ping::PingService;
using ping::RequestType;
using ping::ResponseType;

class PingClient
{
public:
  PingClient(std::shared_ptr<Channel> channel, size_t id)
      : stub_(PingService::NewStub(channel)), id(id) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  void Ping()
  {
    this->stream = stub_->Ping(&context);
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

private:
  std::unique_ptr<PingService::Stub> stub_;
  std::unique_ptr<ClientReaderWriter<PingRequest, PingResponse> > stream;
  ClientContext context;
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
  client.Ping();

  return 0;
}

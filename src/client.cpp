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
using ping::ResponseStatus;

class PingClient {
 public:
   PingClient(std::shared_ptr<Channel> channel, size_t id)
       : stub_(PingService::NewStub(channel)), id(id) {}

   // Assembles the client's payload, sends it and presents the response back
   // from the server.
  void Ping()
  {
    ClientContext context;
    std::shared_ptr<ClientReaderWriter<PingRequest, PingResponse>> stream(stub_->Ping(&context));
    PingRequest request;
    PingResponse response;
    request.set_id(this->id);
    // std::cout << "here client ping #begin" << std::endl;
    stream->Write(request);
    ResponseStatus responseStatus = ResponseStatus::WAIT;
    while (stream->Read(&response) && responseStatus == ResponseStatus::WAIT)
    {
      responseStatus = response.responsestatus();
      std::cout << "here client ping #read " << responseStatus << std::endl;
    }
    if (responseStatus == ResponseStatus::WAIT)
    {
      throw std::runtime_error("unresolved request, stream ended and client's still waiting");
    }

    std::cout << "here client ping #finish" << std::endl;
    Status status = stream->Finish();
    std::cout << "here client ping #finish-2: " << int(status.error_code()) << std::endl;
    if (!status.ok())
    {
      throw std::runtime_error(std::to_string(status.error_code()) + ": " + status.error_message());
    }
  }

 private:
   std::unique_ptr<PingService::Stub> stub_;
   const size_t id;
};

int main(int argc, char** argv) {
  if (argc != 2) {
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

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
using ping::PingRoute;
using ping::PingService;

class PingClient {
 public:
   PingClient(std::shared_ptr<Channel> channel)
       : stub_(PingService::NewStub(channel)) {}

   // Assembles the client's payload, sends it and presents the response back
   // from the server.
  void Ping()
  {
    ClientContext context;
    std::shared_ptr<ClientReaderWriter<PingRoute, PingRoute>> stream(stub_->Ping(&context));
    PingRoute route;
    // std::cout << "here client ping #begin" << std::endl;
    size_t limit = 5, count = 0;
    stream->Write(route);
    while(stream->Read(&route))
    {
      std::cout << "here client ping #read " << count << std::endl;
      if (++count > limit) {
        break;
      }
      stream->Write(route);
    }

    std::cout << "here client ping #finish" << std::endl;
    Status status = stream->Finish();
    std::cout << "here client ping #finish-2: " << int(status.error_code()) << std::endl;
    if (!status.ok())
    {
      std::cout << "ERROR => " << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }

 private:
   std::unique_ptr<PingService::Stub> stub_;
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
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  client.Ping();

  return 0;
}

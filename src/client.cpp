#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ping::PingReply;
using ping::PingRequest;
using ping::PingService;

class PingClient {
 public:
   PingClient(std::shared_ptr<Channel> channel)
       : stub_(PingService::NewStub(channel)) {}

   // Assembles the client's payload, sends it and presents the response back
   // from the server.
   bool Ping()
   {
     // Data we are sending to the server.
     PingRequest request;

     // Container for the data we expect from the server.
     PingReply reply;

     // Context for the client. It could be used to convey extra information to
     // the server and/or tweak certain RPC behaviors.
     ClientContext context;
     // The actual RPC.
     Status status = stub_->Ping(&context, request, &reply);
     // Act upon its status.
     if (status.ok())
     {
       return true;
     }
     else
     {
       std::cout << status.error_code() << ": " << status.error_message()
                 << std::endl;
       return false;
     }
  }

 private:
   std::unique_ptr<PingService::Stub> stub_;
};

int main(int argc, char** argv) {
  std::string target_str = "localhost:50051";
  
  PingClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  bool response = client.Ping();
  std::cout << "ping response: " << response << std::endl;

  return 0;
}

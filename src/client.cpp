#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class Client {
 public:
  Client(std::shared_ptr<grpc::Channel> channel)
      : stub_(example::ExampleService::NewStub(channel)) {}

  void perform() {
    example::DataRequest request;
    example::DataResponse response;

    grpc::ClientContext context;

    //...
  }

 private:
   std::unique_ptr<example::ExampleService::Stub> stub_;
};

int main(int argc, char** argv) {

  const std::string targetStr = "localhost:50051";
  Client client(
      grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials()));

  client.perform();

  return 0;
}

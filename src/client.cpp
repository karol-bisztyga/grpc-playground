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
    std::unique_ptr<grpc::ClientReaderWriter<example::DataRequest, example::DataResponse> > stream = this->stub_->ExchangeData(&context);

    std::string data = "?";
    size_t i = 0;
    while(!data.empty()) {
      std::string dataToSend = "request " + std::to_string(++i);
      request.set_data(dataToSend);
      if (!stream->Write(request)) {
        throw std::runtime_error("writing failed");
      }

      if(!stream->Read(&response)) {
        throw std::runtime_error("reading failed");
      }
      data = response.data();
      std::cout << "sent data [" << dataToSend << "], read data [" << data << "]" << std::endl;
    }
    std::cout << "reading over, terminating" << std::endl;
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

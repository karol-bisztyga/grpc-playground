#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include <memory>

#include "BidiReactor.h"

struct Client
{
  std::unique_ptr<example::ExampleService::Stub> stub;

  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {
  }
};

void performBidi(const Client &client) {
  std::unique_ptr<BidiReactor> reactor = std::make_unique<BidiReactor>(client.stub.get());
  while (!reactor->isDone())
  {
    std::string str;
    std::cout << "enter a message: ";
    std::getline(std::cin, str);
    reactor->NextWrite(str);
  }
}

void performWrite(const Client &client) {}

void performRead(const Client &client) {}

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  std::string opt = "?";
  std::string options = "brwe";
  while(options.find(opt) == std::string::npos) {
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[b] bidi stream" << std::endl;
    std::cout << "[w] write stream" << std::endl;
    std::cout << "[r] read stream" << std::endl;
    std::cout << "[e] exit" << std::endl;

    std::getline(std::cin, opt);

    switch (opt[0])
    {
    case 'b':
    {
      performBidi(client);
      break;
    }
    case 'r':
    {
      break;
    }
    case 'w':
    {
      break;
    }
    case 'e':
    {
      return 0;
    }
    }
    opt = "?";
  }

  return 0;
}
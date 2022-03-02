#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include <memory>

#include "BidiReactor.h"
#include "ReadReactor.h"

struct Client
{
  std::unique_ptr<example::ExampleService::Stub> stub;
  std::unique_ptr<BidiReactor> bidiReactor;
  std::unique_ptr<ReadReactor> readReactor;

  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {
  }

  void initializeBidiReactor() {
    this->bidiReactor.reset(new BidiReactor(stub.get()));
  }

  void initializeReadReactor(const std::string &data) {
    example::DataRequest request;
    request.set_data(data);
    this->readReactor.reset(new ReadReactor(stub.get(), request));
  }
};

void performBidi(Client &client) {
  client.initializeBidiReactor();
  while (!client.bidiReactor->isDone())
  {
    std::string str;
    std::cout << "enter a message: ";
    std::getline(std::cin, str);
    client.bidiReactor->NextWrite(str);
  }
}

void performWrite(Client &client) {
}

void performRead(Client &client) {
  std::string str;
  std::cout << "enter a message: ";
  std::getline(std::cin, str);
  client.initializeReadReactor(str);
}

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  std::string opt = "?";
  std::string options = "brwe";
  while(options.find(opt[0]) == std::string::npos) {
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
      performRead(client);
      break;
    }
    case 'w':
    {
      performWrite(client);
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
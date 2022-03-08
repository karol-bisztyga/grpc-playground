#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include "ClientBidiReactorBase.h"
#include "ClientReadReactorBase.h"
#include "ClientWriteReactorBase.h"

#include <memory>
#include <functional>
#include <chrono>
#include <thread>

class ClientBidiReactor : public ClientBidiReactorBase<example::DataRequest, example::DataResponse>
{
public:
  using ClientBidiReactorBase<example::DataRequest, example::DataResponse>::ClientBidiReactorBase;

  std::unique_ptr<grpc::Status> prepareRequest(example::DataRequest &request, std::shared_ptr<example::DataResponse> previousResponse) override
  {
    if (previousResponse != nullptr)
    {
      std::cout << "got response: [" << previousResponse->data() << "]" << std::endl;
    }
    std::string str;
    std::cout << "enter a message(type 'exit' to end the connection): " << std::endl;
    std::getline(std::cin, str);
    if (str == "exit")
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    request.set_data(str);
    return nullptr;
  }
};

struct Client
{
  std::unique_ptr<example::ExampleService::Stub> stub;

  size_t readCount = 0;

  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {
  }

  std::unique_ptr<ClientBidiReactor> bidiReactor;

  void initializeBidiReactor()
  {
    this->bidiReactor.reset(new ClientBidiReactor());
  }

  bool reactorInProgress()
  {
    if (this->bidiReactor != nullptr && !this->bidiReactor->isDone())
    {
      return true;
    }
    return false;
  }

  // void initializeReadReactor(const std::string &data, std::function<bool(const example::DataResponse &)> clb)
  // {
  //   example::DataRequest request;
  //   request.set_data(data);
  //   this->readReactor.reset(new ReadReactor(stub.get(), request, clb));
  // }

  // void initializeWriteReactor() {
  //   this->writeReactor.reset(new WriteReactor(stub.get()));
  // }
};

void performBidi(Client &client)
{
  client.initializeBidiReactor();
  client.stub->async()->ExchangeData(&client.bidiReactor->context, &(*client.bidiReactor));
  // while (!client.bidiReactor->isDone())
  // {
  client.bidiReactor->nextWrite();
  // }
}

void performWrite(Client &client)
{
  // client.initializeWriteReactor();
  // while (!client.writeReactor->isDone())
  // {
  //   std::string str;
  //   std::cout << "enter a message(type 'exit' to end the connection): ";
  //   std::getline(std::cin, str);
  //   if (str == "exit") {
  //     client.writeReactor->terminate(grpc::Status::OK);
  //   }
  //   client.writeReactor->NextWrite(str);
  // }
  // std::cout << "done writing to server with response: [" << client.writeReactor->response.data() << "]" << std::endl;
}

void performRead(Client &client)
{
  // std::string str;
  // std::cout << "enter a message(type 'exit' to cut the connection prematurely): ";
  // std::getline(std::cin, str);
  // std::function<bool(const example::DataResponse&)> clb = nullptr;
  // if (str == "exit") {
  //   clb = [&client](const example::DataResponse &response) {
  //     std::cout << "CLB: " << client.readCount << std::endl;
  //     return ++client.readCount > 2;
  //   };
  // }
  // client.initializeReadReactor(str, clb);
}

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  std::string opt = "?";
  std::string options = "brwe";
  while (options.find(opt[0]) == std::string::npos)
  {
    if (client.reactorInProgress())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      continue;
    }
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
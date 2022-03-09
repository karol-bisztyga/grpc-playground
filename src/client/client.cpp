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
    std::cout << "enter a message(type 'exit' to end the connection): ";
    std::getline(std::cin, str);
    if (str == "exit")
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    request.set_data(str);
    return nullptr;
  }
};

class ClientReadReactor : public ClientReadReactorBase<example::DataRequest, example::DataResponse>
{
  size_t readCount = 0;
  const size_t limit = 0;

public:
  ClientReadReactor(size_t limit = 0) : limit(limit) {}

  std::unique_ptr<grpc::Status> readResponse(const example::DataResponse &response) override
  {
    std::cout << "CLB: " << response.data() << std::endl;
    ++this->readCount;
    if (this->limit && this->readCount > this->limit)
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    return nullptr;
  }
};

class ClientWriteReactor : public ClientWriteReactorBase<example::DataRequest, example::DataResponse>
{
public:
  std::unique_ptr<grpc::Status> prepareRequest(example::DataRequest &request) override
  {
    std::string str;
    std::cout << "enter a message(type 'exit' to end the connection on the client(no response), provide an empty message to end the connection on the server): ";
    std::getline(std::cin, str);
    if (str == "exit")
    {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    request.set_data(str);
    return nullptr;
  };

  void doneCallback() override
  {
    std::cout << "done writing to server with response: [" << this->response.data() << "]" << std::endl;
  }
};

class UnaryClientReactor : public grpc::ClientUnaryReactor
{
public:
  grpc::ClientContext context;
  example::DataRequest request;
  example::DataResponse response;
  bool done = false;
};

struct Client
{
  std::unique_ptr<example::ExampleService::Stub> stub;

  std::unique_ptr<ClientBidiReactor> bidiReactor;
  std::unique_ptr<ClientReadReactor> readReactor;
  std::unique_ptr<ClientWriteReactor> writeReactor;
  std::unique_ptr<UnaryClientReactor> unaryReactor;

  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {
  }

  void initializeBidiReactor()
  {
    this->bidiReactor.reset(new ClientBidiReactor());
  }

  void initializeReadReactor(size_t limit)
  {
    this->readReactor.reset(new ClientReadReactor(limit));
  }

  void initializeWriteReactor()
  {
    this->writeReactor.reset(new ClientWriteReactor());
  }

  void initializeUnaryReactor()
  {
    this->unaryReactor.reset(new UnaryClientReactor());
  }

  bool reactorInProgress()
  {
    if (this->bidiReactor != nullptr && !this->bidiReactor->isDone())
    {
      return true;
    }
    if (this->readReactor != nullptr && !this->readReactor->isDone())
    {
      return true;
    }
    if (this->writeReactor != nullptr && !this->writeReactor->isDone())
    {
      return true;
    }
    if (this->unaryReactor != nullptr && !this->unaryReactor->done)
    {
      return true;
    }
    return false;
  }
};

void performBidi(Client &client)
{
  client.initializeBidiReactor();
  client.stub->async()->ExchangeData(&client.bidiReactor->context, &(*client.bidiReactor));
  client.bidiReactor->nextWrite();
}

void performWrite(Client &client)
{
  client.initializeWriteReactor();
  client.stub->async()->OneWayStreamClientToServer(&client.writeReactor->context, &client.writeReactor->response, &(*client.writeReactor));
  client.writeReactor->nextWrite();
}

void performRead(Client &client)
{
  std::string str;
  std::cout << "enter a message(type 'exit' to cut the connection prematurely): ";
  std::getline(std::cin, str);
  size_t limit = 0;
  if (str == "exit")
  {
    limit = 2;
  }
  client.initializeReadReactor(limit);
  client.readReactor->request.set_data(str);
  client.stub->async()->OneWayStreamServerToClient(&client.readReactor->context, &client.readReactor->request, &(*client.readReactor));
  client.readReactor->start();
}

void performUnary(Client &client)
{
  std::cout << "enter a message: ";
  std::string str;
  std::getline(std::cin, str);
  client.initializeUnaryReactor();
  client.unaryReactor->request.set_data(str);
  client.stub->async()->Unary(&client.unaryReactor->context, &client.unaryReactor->request, &client.unaryReactor->response, [&client](grpc::Status status)
                              {
                                client.unaryReactor->done = true;
                                if (!status.ok()) {
                                  std::cout << "unary error: " + status.error_message() << std::endl;
                                  return;
                                }
                                std::cout << "unary response: " << client.unaryReactor->response.data() << std::endl; });
}

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  std::string opt = "?";
  std::string options = "brwoe";
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
    std::cout << "[o] one message" << std::endl;
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
    case 'o':
    {
      performUnary(client);
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
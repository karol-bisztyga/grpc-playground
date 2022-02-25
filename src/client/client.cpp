#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

#include <memory>

class Reactor : public grpc::ClientBidiReactor<example::DataRequest, example::DataResponse>
{
  grpc::ClientContext context;
  example::DataRequest request;
  example::DataResponse response;
  grpc::Status status;
  bool done = false;
  size_t counter = 0;

public:
  Reactor(example::ExampleService::Stub *stub)
  {
    stub->async()->ExchangeData(&this->context, this);
  }

  void NextWrite(std::string msg)
  {
    if (this->counter && this->response.data().empty())
    {
      std::cout << "empty message - terminating" << std::endl;
      this->StartWritesDone();
      return;
    }
    this->request.set_data(msg);
    StartWrite(&this->request);
    if (!this->counter)
    {
      StartCall();
    }
    ++this->counter;
  }

  bool isDone() {
    return this->done;
  }

  void OnWriteDone(bool ok) override
  {
    std::cout << "Done writing: " << this->request.data() << std::endl;
    StartRead(&this->response);
  }

  void OnReadDone(bool ok) override
  {
    if (!ok)
    {
      std::cout << "error - terminating" << std::endl;
      this->StartWritesDone();
      return;
    }
    std::cout << "Got message [" << this->response.data() << "]" << std::endl;
  }

  void OnDone(const grpc::Status &status) override
  {
    std::cout << "- HERE OnDone" << std::endl;
    std::cout << "DONE" << std::endl;
    this->status = status;
    this->done = true;
  }
};

class Client
{
public:
  std::unique_ptr<Reactor> reactor;

  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {
    this->reactor = std::make_unique<Reactor>(this->stub.get());
  }
private:
  std::unique_ptr<example::ExampleService::Stub> stub;
};

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  while (!client.reactor->isDone())
  {
    std::string str;
    std::cout << "enter a fake message: ";
    std::getline(std::cin, str);
    if (client.reactor->isDone()) {
      std::cout << "connection lost, aborting" << std::endl;
      break;
    }
    std::cout << "enter a real message: ";
    std::getline(std::cin, str);
    client.reactor->NextWrite(str);
  }

  return 0;
}
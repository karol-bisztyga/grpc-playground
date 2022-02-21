#include <grpcpp/grpcpp.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

class Reactor : public grpc::ClientBidiReactor<example::DataRequest, example::DataResponse>
{
  grpc::ClientContext context;
  example::DataRequest request;
  example::DataResponse response;
  std::mutex mtx;
  std::condition_variable cv;
  grpc::Status status;
  bool done = false;
  size_t counter = 0;

  void NextWrite()
  {
    if (this->counter && this->response.data().empty())
    {
      std::cout << "empty message - terminating" << std::endl;
      this->StartWritesDone();
      return;
    }
    this->request.set_data("hello " + std::to_string(++this->counter));
    StartWrite(&this->request);
  }

public:
  Reactor(example::ExampleService::Stub *stub)
  {
    stub->async()->ExchangeData(&this->context, this);
    NextWrite();
    StartCall();
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
    NextWrite();
  }

  void OnDone(const grpc::Status &status) override
  {
    std::cout << "- HERE OnDone" << std::endl;
    std::cout << "DONE" << std::endl;
    std::unique_lock<std::mutex> l(this->mtx);
    this->status = status;
    this->done = true;
    this->cv.notify_one();
  }

  grpc::Status Await()
  {
    std::unique_lock<std::mutex> l(this->mtx);
    this->cv.wait(l, [this]
                  { return this->done; });
    return std::move(this->status);
  }
};

class Client
{
public:
  Client(std::shared_ptr<grpc::Channel> channel)
      : stub(example::ExampleService::NewStub(channel))
  {}

  void ExchangeData()
  {
    Reactor reactor(this->stub.get());
    grpc::Status status = std::move(reactor.Await());
    if (!status.ok()) {
      std::cout << "Rpc failed: " << status.error_message() << std::endl;
    }
  }

private:
  std::unique_ptr<example::ExampleService::Stub> stub;
};

int main(int argc, char **argv)
{
  Client client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  client.ExchangeData();

  return 0;
}
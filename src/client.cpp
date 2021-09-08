#include <iostream>
#include <memory>
#include <string>

#include <chrono>

#include <grpcpp/grpcpp.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using namespace std::chrono;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using ping::CheckRequest;
using ping::CheckResponse;
using ping::CheckResponseType;
using ping::NewPrimaryRequest;
using ping::NewPrimaryResponse;
using ping::PingService;
using ping::PongRequest;
using ping::PongResponse;

class PingClient
{
public:
  PingClient(std::shared_ptr<Channel> channel, size_t id, long long deviceToken)
      : stub_(PingService::NewStub(channel)),
        id(id),
        deviceToken(deviceToken) {}

  CheckResponseType checkIfPrimaryDeviceOnline()
  {
    ClientContext context;
    CheckRequest request;
    CheckResponse response;

    request.set_id(this->id);
    request.set_devicetoken(this->deviceToken);

    Status status = stub_->CheckIfPrimaryDeviceOnline(&context, request, &response);
    if (!status.ok())
    {
      throw std::runtime_error(status.error_message());
    }
    return response.checkresponsetype();
  }

  bool becomeNewPrimaryDevice()
  {
    ClientContext context;
    NewPrimaryRequest request;
    NewPrimaryResponse response;

    request.set_id(this->id);
    request.set_devicetoken(this->deviceToken);

    Status status = stub_->BecomeNewPrimaryDevice(&context, request, &response);
    if (!status.ok())
    {
      throw std::runtime_error(status.error_message());
    }
    return response.success();
  }

  void sendPong()
  {
    ClientContext context;
    PongRequest request;
    PongResponse response;

    request.set_id(this->id);
    request.set_devicetoken(this->deviceToken);

    Status status = stub_->SendPong(&context, request, &response);
    if (!status.ok())
    {
      throw std::runtime_error(status.error_message());
    }
  }

private:
  std::unique_ptr<PingService::Stub> stub_;
  const size_t id;
  const long long deviceToken;
};

// this is a simulation of a device token
// I figured we can use a timestamp in milliseconds as a unique id for a process
// I'm going to assume it's not possible to launch multiple processes
// at the same very millisecond
long long generateUID()
{
  milliseconds ms = duration_cast<milliseconds>(
      system_clock::now().time_since_epoch());
  return ms.count();
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "please provide an argument with a client id" << std::endl;
    return 1;
  }
  size_t id = (size_t)atoi(argv[1]);
  long long deviceToken = generateUID();
  std::cout << "client start" << std::endl;
  std::cout << "id           : " << id << std::endl;
  std::cout << "device token : " << deviceToken << std::endl;
  std::string target_str = "localhost:50051";

  PingClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()),
      id,
      deviceToken);

  char option = '?';
  while (option != 'e')
  {
    std::string options = "cbse";
    std::cout << "what you want to do?" << std::endl;
    std::cout << "[c] check if primary device is online" << std::endl;
    std::cout << "[b] become a new primary device" << std::endl;
    std::cout << "[s] send pong" << std::endl;
    std::cout << "[e] exit" << std::endl;
    std::cin >> option;
    if (options.find(option) == std::string::npos)
    {
      std::cout << "invalid command [" << option << "], skipping" << std::endl;
    }
    try
    {
      switch (option)
      {
      case 'c':
      {
        CheckResponseType checkResponse = client.checkIfPrimaryDeviceOnline();
        std::cout << "check primary device response: " << ping::CheckResponseType_Name(checkResponse) << std::endl;
        break;
      }
      case 'b':
      {
        bool success = client.becomeNewPrimaryDevice();
        std::cout << "trying to become a new primary device... ";
        if (success)
        {
          std::cout << "success";
        }
        else
        {
          std::cout << "failed";
        }
        std::cout << std::endl;
        break;
      }
      case 's':
      {
        std::cout << "sending pong... ";
        client.sendPong();
        std::cout << "sent!" << std::endl;
        break;
      }
      }
    }
    catch (std::runtime_error &e)
    {
      std::cout << "error: " << e.what() << std::endl;
    }
  }

  return 0;
}

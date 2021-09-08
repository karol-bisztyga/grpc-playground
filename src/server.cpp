#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <chrono>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

#include "threadSafeQueue.h"

using namespace std::chrono_literals;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using ping::CheckRequest;
using ping::CheckResponse;
using ping::CheckResponseType;
using ping::PongRequest;
using ping::PongResponse;
using ping::PingService;
using ping::NewPrimaryRequest;
using ping::NewPrimaryResponse;

using std::size_t;

struct ClientData
{
  const size_t id;
  ThreadSafeQueue<bool> pingRequests;
  std::mutex mutex;
  std::condition_variable cv;
  bool isActive = false;
  long long deviceToken;

  ClientData(const size_t id, long long deviceToken) : id(id), deviceToken(deviceToken) {}
};

class ThreadSafeMap
{
  std::mutex mutex;
  std::unordered_map<size_t, std::shared_ptr<ClientData> > map;

public:
  std::shared_ptr<ClientData> get(const size_t id)
  {
    std::unique_lock<decltype(mutex)> lock(mutex);
    std::unordered_map<size_t, std::shared_ptr<ClientData> >::iterator it = this->map.find(id);
    if (it == this->map.end())
    {
      return nullptr;
    }
    return it->second;
  }

  void set(const size_t id, std::shared_ptr<ClientData> clientData)
  {
    std::unique_lock<decltype(mutex)> lock(mutex);
    this->map.insert(std::make_pair(id, clientData));
  }
};

class PingServiceImpl final : public PingService::Service
{
public:
  ThreadSafeMap primaries;

  Status CheckIfPrimaryDeviceOnline(ServerContext *context, const CheckRequest *request, CheckResponse *response) override
  {
    const size_t id = (size_t)request->id();
    const long long deviceToken = (long long)request->devicetoken();
    std::cout << "check if primary device is online/" << id << "/" << deviceToken << std::endl;

    if (primaries.get(id) == nullptr)
    {
      // this device becomes a new primary device
      response->set_checkresponsetype(CheckResponseType::PRIMARY_DOESNT_EXIST);
    }
    else if (deviceToken == primaries.get(id)->deviceToken)
    {
      response->set_checkresponsetype(CheckResponseType::CURRENT_IS_PRIMARY);
    }
    else
    {
      // TODO: the background notif should be sent what cannot be really simulated here I believe
      primaries.get(id)->pingRequests.enqueue(true);

      // check for a pong with a timeout...
      std::unique_lock<std::mutex> lock(primaries.get(id)->mutex);
      // TODO: timeout currently set for 3s, to be changed
      if (primaries.get(id)->cv.wait_for(lock, 3000ms, [=]
                                         { return primaries.get(id)->isActive; }))
      {
        std::cout << "got a response " << primaries.get(id)->isActive << std::endl;
        response->set_checkresponsetype(CheckResponseType::PRIMARY_ONLINE);
      }
      else
      {
        std::cout << "timed out" << std::endl;
        response->set_checkresponsetype(CheckResponseType::PRIMARY_OFFLINE);
      }
      // reset isActive to false so next time a pong is also required
      primaries.get(id)->isActive = false;
      primaries.get(id)->cv.notify_all();
    }

    return Status::OK;
  }

  Status BecomeNewPrimaryDevice(ServerContext* context, const NewPrimaryRequest* request, NewPrimaryResponse* response) override
  {
    const size_t id = (size_t)request->id();
    const long long deviceToken = (long long)request->devicetoken();
    std::cout << "become a new primary device/" << id << "/" << deviceToken << std::endl;

    if (primaries.get(id) == nullptr)
    {
      std::shared_ptr<ClientData> clientData = std::make_shared<ClientData>(
        (size_t)request->id(),
        deviceToken
      );
      primaries.set(id, clientData);
      response->set_success(true);
    }
    else
    {
      response->set_success(false);
    }
    return Status::OK;
  }

  Status SendPong(ServerContext* context, const PongRequest* request, PongResponse* response) override
  {
    const size_t id = (size_t)request->id();
    const long long deviceToken = (long long)request->devicetoken();
    std::cout << "send pong/" << id << "/" << deviceToken << std::endl;

    if (primaries.get(id) == nullptr || primaries.get(id)->deviceToken != deviceToken)
    {
      std::cout << "received pong from non-primary device, noop" << std::endl;
      return Status::OK;
    }

    primaries.get(id)->pingRequests.dequeue();

    primaries.get(id)->isActive = true;
    primaries.get(id)->cv.notify_all();
    
    return Status::OK;
  }
};

void RunServer()
{
  std::string server_address("localhost:50051");
  PingServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();

  return 0;
}

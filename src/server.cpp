#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <folly/MPMCQueue.h>

#include <grpcpp/grpcpp.h>

#include <chrono>

#include "../_generated/ping.grpc.pb.h"
#include "../_generated/ping.pb.h"

#include "threadSafeQueue.h"

using namespace std::chrono_literals;

using std::size_t;

enum class ClientState
{
  ONLINE,
  OFFLINE,
};

struct ClientData
{
  const std::string id;
  const std::string deviceToken;

  // folly::MPMCQueue<bool> pingRequests2;
  ThreadSafeQueue<bool> pingRequests;
  std::mutex mutex;
  std::condition_variable cv;
  bool isActive = false;
  ClientState lastState = ClientState::ONLINE;

  ClientData(const std::string id, const std::string deviceToken)
      : id(id), deviceToken(deviceToken) {}
};

class ThreadSafeMap
{
  std::mutex mutex;
  std::unordered_map<std::string, std::shared_ptr<ClientData> > map;

public:
  std::shared_ptr<ClientData> get(const std::string id)
  {
    std::unique_lock<decltype(mutex)> lock(mutex);
    std::unordered_map<std::string, std::shared_ptr<ClientData> >::iterator it =
        this->map.find(id);
    if (it == this->map.end())
    {
      return nullptr;
    }
    return it->second;
  }

  void set(const std::string id, std::shared_ptr<ClientData> clientData)
  {
    std::unique_lock<decltype(mutex)> lock(mutex);
    this->map[id] = clientData;
  }
};

class PingServiceImpl final : public ping::PingService::Service
{
public:
  ThreadSafeMap primaries;

  grpc::Status
  CheckIfPrimaryDeviceOnline(grpc::ServerContext *context,
                             const ping::CheckRequest *request,
                             ping::CheckResponse *response) override
  {
    const std::string id = request->id();
    const std::string deviceToken = request->devicetoken();
    std::cout << "check if primary device is online/" << id << "/"
              << deviceToken << std::endl;

    if (primaries.get(id) == nullptr)
    {
      response->set_checkresponsetype(
          ping::CheckResponseType::PRIMARY_DOESNT_EXIST);
    }
    else if (deviceToken == primaries.get(id)->deviceToken)
    {
      response->set_checkresponsetype(
          ping::CheckResponseType::CURRENT_IS_PRIMARY);
    }
    else
    {
      // TODO: the background notif should be sent what cannot be really
      // simulated here I believe
      primaries.get(id)->pingRequests.enqueue(true);

      // check for a pong with a timeout...
      std::unique_lock<std::mutex> lock(primaries.get(id)->mutex);
      // TODO: timeout currently set for 3s, to be changed
      if (primaries.get(id)->cv.wait_for(
              lock, 3000ms, [=]
              { return primaries.get(id)->isActive; }))
      {
        std::cout << "got a response " << primaries.get(id)->isActive
                  << std::endl;
        primaries.get(id)->lastState = ClientState::ONLINE;
        response->set_checkresponsetype(
            ping::CheckResponseType::PRIMARY_ONLINE);
      }
      else
      {
        std::cout << "timed out" << std::endl;
        primaries.get(id)->lastState = ClientState::OFFLINE;
        response->set_checkresponsetype(
            ping::CheckResponseType::PRIMARY_OFFLINE);
      }
      // reset isActive to false so next time a pong is also required
      primaries.get(id)->isActive = false;
      primaries.get(id)->cv.notify_all();
    }

    return grpc::Status::OK;
  }

  grpc::Status
  BecomeNewPrimaryDevice(grpc::ServerContext *context,
                         const ping::NewPrimaryRequest *request,
                         ping::NewPrimaryResponse *response) override
  {
    const std::string id = request->id();
    const std::string deviceToken = request->devicetoken();
    std::cout << "become a new primary device/" << id << "/" << deviceToken
              << std::endl;

    std::shared_ptr<ClientData> clientData =
        std::make_shared<ClientData>(id, deviceToken);
    if (primaries.get(id) == nullptr)
    {
      primaries.set(id, clientData);
      response->set_success(true);
    }
    else
    {
      if (primaries.get(id)->lastState == ClientState::ONLINE)
      {
        response->set_success(false);
      }
      else
      {
        primaries.set(id, clientData);
        response->set_success(true);
      }
    }
    return grpc::Status::OK;
  }

  grpc::Status SendPong(grpc::ServerContext *context,
                        const ping::PongRequest *request,
                        ping::PongResponse *response) override
  {
    const std::string id = request->id();
    const std::string deviceToken = request->devicetoken();
    std::cout << "send pong/" << id << "/" << deviceToken << std::endl;

    if (primaries.get(id) == nullptr ||
        primaries.get(id)->deviceToken != deviceToken)
    {
      std::cout << "received pong from non-primary device, noop" << std::endl;
      return grpc::Status::OK;
    }

    primaries.get(id)->pingRequests.dequeue();

    primaries.get(id)->isActive = true;
    primaries.get(id)->cv.notify_all();

    return grpc::Status::OK;
  }
};

void RunServer()
{
  std::string server_address = "0.0.0.0:50051";
  PingServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
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

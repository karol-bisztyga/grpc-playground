#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <chrono>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

#include "threadSafeQueue.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using ping::HostPingRequest;
using ping::HostPingResponse;
using ping::InitialRequest;
using ping::InitialResponse;
using ping::InitialResponseType;
using ping::PingService;
using ping::SendPingRequest;
using ping::SendPingResponse;
using ping::SendPingResponseType;

using std::size_t;

struct ClientData
{
  const size_t id;
  ThreadSafeQueue<bool> pingRequests;
  std::mutex mutex;
  std::condition_variable cv;
  bool isActive = false;

  ClientData(const size_t id) : id(id) {}
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

  Status Initialize(ServerContext *context, const InitialRequest *request, InitialResponse *response) override
  {
    std::cout << "initialize" << std::endl;
    const size_t id = (size_t)request->id();

    if (primaries.get(id) == nullptr)
    {
      // this device becomes a new primary device
      response->set_initialresponsetype(InitialResponseType::NEW_PRIMARY);

      std::shared_ptr<ClientData> clientData = std::make_shared<ClientData>((size_t)request->id());
      primaries.set(id, clientData);
    }
    else
    {
      response->set_initialresponsetype(InitialResponseType::PRIMARY_PRESENT);
    }

    return Status::OK;
  }

  Status SendPing(ServerContext *context, const SendPingRequest *request, SendPingResponse *response) override
  {
    using namespace std::chrono_literals;
    std::cout << "send ping" << std::endl;
    const size_t id = (size_t)request->id();
    // add a ping request for the client that owns a primary device
    primaries.get(id)->pingRequests.enqueue(true);

    // check for a pong with a timeout...
    std::unique_lock<std::mutex> lock(primaries.get(id)->mutex);
    if (primaries.get(id)->cv.wait_for(lock, 500ms, [=]
                                                   { return primaries.get(id)->isActive; }))
    {
      std::cout << "done waiting " << primaries.get(id)->isActive << std::endl;
      response->set_sendpingresponsetype(SendPingResponseType::PRIMARY_ONLINE);
    }
    else
    {
      std::cout << "timed out" << std::endl;
      response->set_sendpingresponsetype(SendPingResponseType::PRIMARY_OFFLINE);
    }
    return Status::OK;
  }

  Status HostPing(ServerContext *context, ServerReaderWriter<HostPingResponse, HostPingRequest> *stream) override
  {
    std::cout << "host ping" << std::endl;

    HostPingRequest request;
    HostPingResponse response;
    stream->Read(&request);
    const size_t id = (size_t)request.id();
    // wait for ping requests from other threads here
    while (primaries.get(id)->pingRequests.dequeue())
    {
      // send a ping to a client
      stream->Write(response);

      stream->Read(&request);
      primaries.get(id)->isActive = true;
      primaries.get(id)->cv.notify_all();
    }

    return Status::OK;
  }
};

void RunServer()
{
  std::string server_address("localhost:50051");
  PingServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
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

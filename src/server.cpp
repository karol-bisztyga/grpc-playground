#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <chrono>
using namespace std::chrono_literals;

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

#include "threadSafeQueue.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;

using ping::PingRequest;
using ping::PingResponse;
using ping::PingService;
using ping::RequestType;
using ping::ResponseType;

using std::size_t;

struct ClientData {
  const size_t id;
  ThreadSafeQueue<bool> pingRequests;
  std::mutex mutex;
  std::condition_variable cv;
  bool isActive = false;

  ClientData(const size_t id): id(id) {}
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

// Logic and data behind the server's behavior.
class PingServiceImpl final : public PingService::Service
{
public:
  ThreadSafeMap primaries;

  // every connection goes on the separate thread
  Status Ping(ServerContext *context, ServerReaderWriter<PingResponse, PingRequest> *reactor) override
  {
    std::cout << "here server ping #start " << std::this_thread::get_id() << std::endl;

    std::mutex m;
    std::condition_variable cv;

    PingRequest request;
    PingResponse response;
    reactor->Read(&request);

    std::shared_ptr<ClientData> clientData = std::make_shared<ClientData>((size_t)request.id());

    std::cout << "here server ping #read, client id: " << clientData->id << std::endl;

    if (primaries.get(clientData->id) == nullptr)
    {
      // this device becomes a new primary device
      response.set_responsetype(ResponseType::NEW_PRIMARY);
      reactor->Write(response);

      primaries.set(clientData->id, clientData);

      // wait for ping requests from other threads here
      while (clientData->pingRequests.dequeue())
      {
        // send a ping to a client
        response.set_responsetype(ResponseType::PING);
        reactor->Write(response);

        reactor->Read(&request);
        if (request.requesttype() == RequestType::PONG) {
          clientData->isActive = true;
          clientData->cv.notify_all();
        }
      }
    }
    else
    {
      // there's another primary device registered already
      // current client has to wait for the pong from the client
      // that owns the current primary device
      response.set_responsetype(ResponseType::WAIT);
      reactor->Write(response);

      // add a ping request for the client that owns a primary device
      primaries.get(clientData->id)->pingRequests.enqueue(true);

      // check for a pong with a timeout...
      std::cout << "waiting for the pong from the primary device" << std::endl;
      std::unique_lock<std::mutex> lock(primaries.get(clientData->id)->mutex);
      if(primaries.get(clientData->id)->cv.wait_for(lock, 500ms, [=]{
        return primaries.get(clientData->id)->isActive;
      })) {
        std::cout << "done waiting " << primaries.get(clientData->id)->isActive << std::endl;

        response.set_responsetype(ResponseType::PRIMARY_ONLINE);
        reactor->Write(response);
      } else {
        std::cout << "timed out" << std::endl;

        response.set_responsetype(ResponseType::PRIMARY_OFFLINE);
        reactor->Write(response);
      }
      // TODO: the question here is: should we terminate the client or leave it
      // so it becomes a primary device once the original one perishes?
      // to do that we'd probably need an additional queue of primary candidates
      // and a non busy loop here(also some additional fields in ResponseType)
      // for now it terminates
    }
    std::cout << "terminating client" << std::endl;
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

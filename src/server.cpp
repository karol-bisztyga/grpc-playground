#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using ping::PingRoute;
using ping::PingService;

// Logic and data behind the server's behavior.
class PingServiceImpl final : public PingService::Service {
  Status Ping(ServerContext *context, ServerReaderWriter<PingRoute, PingRoute> *stream) override
  {
    std::cout << "here server ping #start" << std::endl;
    PingRoute route;
    size_t limit = 3, count = 0;
    while(stream->Read(&route))
    {
      std::cout << "here server ping #read " << count << std::endl;
      if (++count > limit) {
        break;
      }
      stream->Write(route);
    }
    std::cout << "here server ping #end" << std::endl;
    return Status::OK;
  }
};

void RunServer() {
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

int main(int argc, char** argv) {
  RunServer();

  return 0;
}

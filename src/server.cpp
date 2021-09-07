#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "../_generated/ping.pb.h"
#include "../_generated/ping.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using ping::PingRequest;
using ping::PingResponse;
using ping::ResponseStatus;
using ping::PingService;

// Logic and data behind the server's behavior.
class PingServiceImpl final : public PingService::Service {
public:
  Status Ping(ServerContext *context, ServerReaderWriter<PingResponse, PingRequest> *reactor) override
  {
    std::cout << "here server ping #start" << std::endl;
    PingRequest request;
    PingResponse response;
    reactor->Read(&request);

    std::cout << "here server ping #read, client id: " << request.id() << std::endl;
    for (size_t i = 0; i < 3; ++i)
    {
      response.set_responsestatus(ResponseStatus::WAIT);
      reactor->Write(response);
    }

    response.set_responsestatus(ResponseStatus::NEW_PRIMARY);
    reactor->Write(response);
    std::cout << "here server ping #end" << std::endl;
    return Status::OK;
  }
};

void RunServer() {
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

int main(int argc, char** argv) {
  RunServer();

  return 0;
}

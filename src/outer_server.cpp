#include "OuterServiceImpl.h"

#include "Constants.h"
#include "ThreadSafeQueue.h"
#include "ServiceClient.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

void RunServer(ThreadSafeQueue<std::shared_ptr<TalkBetweenServicesReactor>> &reactorsQueue) {
  OuterServiceImpl service(&reactorsQueue);

  std::string addr = LISTEN_ADDRESS + ":" + OUTER_SERVER_PORT;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(
      addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
            << "]"
            << "server listening at :" << addr << std::endl;

  server->Wait();
}

int main(int argc, char **argv) {
  ThreadSafeQueue<std::shared_ptr<TalkBetweenServicesReactor>> reactorsQueue;

  std::thread th([&reactorsQueue](){
    while(true) {
      std::cout
          << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
          << "][main::lambda] loop"
          << std::endl;
      std::shared_ptr<TalkBetweenServicesReactor> reactor = reactorsQueue.dequeue();
      std::cout
          << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
          << "][main::lambda] dequeued"
          << std::endl;
      ServiceClient::getInstance().talk(reactor);
      std::cout
          << "[" << std::hash<std::thread::id>{}(std::this_thread::get_id())
          << "][main::lambda] invoked talk"
          << std::endl;
    }
  });

  RunServer(reactorsQueue);

  return 0;
}

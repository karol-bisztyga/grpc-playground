#include "Tools.h"
#include "Constants.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#define MB 1024 * 1024

int main(int argc, char **argv)
{
  std::string targetAddr = LISTEN_ADDRESS + ":" + OUTER_SERVER_PORT;
  auto channel = grpc::CreateChannel(
      targetStr,
      grpc::InsecureChannelCredentials());
  std::cout << "client start on: " << targetAddr << std::endl;

  try
  {
  }
  catch (std::runtime_error &e)
  {
    std::cout << "error: " << e.what() << std::endl;
  }

  return 0;
}

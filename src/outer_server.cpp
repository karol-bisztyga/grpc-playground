#include "Tools.h"

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#define MB 1024 * 1024

int main(int argc, char **argv)
{
  try
  {
  }
  catch (std::runtime_error &e)
  {
    std::cout << "error: " << e.what() << std::endl;
  }

  return 0;
}

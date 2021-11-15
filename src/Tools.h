#pragma once

#include <string>
#include <random>

int randomNumber(const int from, const int to)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(from, to);

  return dist(mt);
}

std::string randomString(size_t size = 20)
{
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  std::string result;

  for (size_t i = 0; i < size; ++i)
  {
    result += str[randomNumber(0, str.size() - 1)];
  }

  return result;
}

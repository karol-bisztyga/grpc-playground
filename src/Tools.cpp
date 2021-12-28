#include "Tools.h"

int randomNumber(const int from, const int to)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(from, to);

  return dist(mt);
}

std::string randomString(size_t size)
{
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  std::string result;

  for (size_t i = 0; i < size; ++i)
  {
    result += str[randomNumber(0, str.size() - 1)];
  }

  return result;
}

long long currentTimestamp()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(
             system_clock::now().time_since_epoch())
      .count();
}

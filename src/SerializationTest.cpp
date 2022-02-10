#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <grpcpp/grpcpp.h>
#include <google/protobuf/message.h>

#include "../_generated/example.pb.h"
#include "../_generated/example.grpc.pb.h"

void printSerializableData(example::SerializableData data) {
  std::cout << "DATA:" << std::endl;
  std::cout << "  name: [" << data.name() << "]" << std::endl;
  std::cout << "  temperature: [" << data.temperature() << "]" << std::endl;
  std::cout << "  age: [" << data.age() << "]" << std::endl;
  std::cout << "  gender: [" << data.gender() << "]" << std::endl;
  std::cout << "  secret: [" << data.secret() << "]" << std::endl;
}

std::string serialize(const example::SerializableData data) {
  std::string result;
  if (!data.SerializeToString(&result)) {
    throw std::runtime_error("serialization error");
  }
  return result;
}

example::SerializableData deserialize(const std::string data) {
  example::SerializableData result;
  if (!result.ParseFromString(data)) {
    throw std::runtime_error("deserialization error");
  }
  return result;
}

int main(int argc, char** argv) {
  example::SerializableData data;
  data.set_name("john");
  data.set_temperature(36.6);
  data.set_age(23);
  data.set_gender(true);
  data.set_secret("JLkuJxj8M8pm+0ahLmCZk5u6t5uVdcVDna5pqzfQG+b/bJz1KyZwRzb3v4veSfCyLK3Eruqp2+yVVbOlSuh4GyXxiqGOX/5nvFJUYulXg2AoX9plQXvxq9adYSgnaKKtGJn3gApjsXMI8CFVmjVS3ZMCFT3N7N+A+quRYYkv7BY=");

  // std::cout << "SERIALIZATION:" << std::endl;
  // std::cout << "original data:" << std::endl;
  // printSerializableData(data);

  // serialize
  const std::string serializedData = serialize(data);
  // std::cout << "serialized data: [" << serializedData << "]" << std::endl;

  // deserialize
  example::SerializableData deserializedData = deserialize(serializedData);
  // std::cout << "deserialized data:" << std::endl;
  // printSerializableData(deserializedData);

  assert(data.name() == deserializedData.name());
  assert(data.temperature() == deserializedData.temperature());
  assert(data.age() == deserializedData.age());
  assert(data.gender() == deserializedData.gender());
  assert(data.secret() == deserializedData.secret());

  std::cout << "SUCCESS: all checks passed, deserialized data is the same as the original one" << std::endl;

  return 0;
}

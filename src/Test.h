#pragma once

#include "Client.h"
#include "Tools.h"

#include <grpcpp/grpcpp.h>

#include <iostream>

void backupTestAssert(bool cond, const std::string &label) {
  if (!cond) {
    throw std::runtime_error("assertion failed: " + label);
  }
}

void doTest() {
  // preparing the data
  //  creating a key
  std::string key = randomString();
  //  creating a compaction
  std::vector<std::string> compaction;
  const size_t nCompacts = randomNumber(3, 7);
  for (size_t i = 0; i < nCompacts; ++i)
  {
    std::string chunk = randomString();
    compaction.push_back(chunk);
  }
  //  creating some logs
  std::vector<std::string> logs;
  const size_t nLogs = randomNumber(3, 7);
  for (size_t i = 0; i < nLogs; ++i)
  {
    std::string chunk = randomString();
    logs.push_back(chunk);
  }

  // creating the client
  Client client(
      grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()),
      "1");

  // pushing the data to the server
  client.resetKey(key, compaction);
  for (size_t i = 0; i < logs.size(); ++i) {
    client.sendLog(logs[i]);
  }

  // retreiving the data from the server
  CompactionResponse response = client.pullCompact();
  std::cout << "data from server compaction: " << response.compaction << std::endl;
  std::cout << "data from server logs: " << response.logs << std::endl;
  std::string pulledKey = client.pullBackupKey("");

  // checking data consistency
  size_t offset = 0;
  for (size_t i = 0; i < compaction.size(); ++i) {
    const size_t size = compaction[i].size();
    backupTestAssert(compaction[i] == response.compaction.substr(offset, size), "compaction chunk " + std::to_string(offset) + "-" + std::to_string(size));
    offset += size;
  }
  backupTestAssert(offset == response.compaction.size(), "whole compaction data read");

  offset = 0;
  for (size_t i = 0; i < logs.size(); ++i) {
    const size_t size = logs[i].size();
    backupTestAssert(logs[i] == response.logs.substr(offset, size), "logs chunk " + std::to_string(offset) + "-" + std::to_string(size));
    offset += size;
  }
  backupTestAssert(offset == response.logs.size(), "whole logs data read");

  backupTestAssert(key == pulledKey, "keys match");
}

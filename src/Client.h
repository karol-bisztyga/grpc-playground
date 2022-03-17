#pragma once

#include "AuthenticationManager.h"
#include "ClientBidiReactorBase.h"
#include "ClientWriteReactorBase.h"
#include "ClientReadReactorBase.h"
#include "Tools.h"

#include "../_generated/backup.pb.h"
#include "../_generated/backup.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

/*
flow:
REGISTRATION
client => PakeRegistrationRequestAndUserID => server
server => pakeRegistrationResponse => client
REGISTRATION_UPLOAD
client => pakeRegistrationUpload => server
server => pakeRegistrationSuccess => client
CREDENTIAL
client => pakeCredentialRequest => server
server => pakeCredentialResponse => client
CREDENTIAL_FINALIZATION
client => pakeCredentialFinalization => server
server => pakeServerMAC => client
MAC_EXCHANGE
client => pakeClientMAC => server
server => empty response => client

authentication is done

SEND_KEY_ENTROPY
client => keyEntropy => server
SEND_DATA_HASH
client => dataHash => server
SEND_CHUNKS
client => data chunk 0 => server
client => data chunk 1 => server
client => data chunk ... => server
client => data chunk n => server

end connection
*/

class CreateNewBackupReactor : public ClientBidiReactorBase<backup::CreateNewBackupRequest, backup::CreateNewBackupResponse>
{
  enum class State {
    AUTHENTICATION = 1,
    KEY_ENTROPY = 2,
    DATA_HASH = 3,
    CHUNKS = 4,
  };
  std::unique_ptr<AuthenticationManager> authenticationManager;
  const size_t chunkLimit;
  size_t currentChunk = 0;
  State state = State::AUTHENTICATION;
public:
  CreateNewBackupReactor(AuthenticationType authenticationType, size_t chunkLimit) : chunkLimit(chunkLimit) {
    this->authenticationManager = std::make_unique<AuthenticationManager>(authenticationType);
    std::cout << "create new backup init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::CreateNewBackupRequest &request, std::shared_ptr<backup::CreateNewBackupResponse> previousResponse) override
  {
    std::cout << "here prepare request" << std::endl;
    if (this->authenticationManager->getState() == AuthenticationState::FAIL) {
      throw std::runtime_error("authentication failed");
    }
    if (this->authenticationManager->getState() == AuthenticationState::IN_PROGRESS) {
    std::cout << "here prepare request auth" << std::endl;
      backup::FullAuthenticationRequestData *authRequest;
      if (previousResponse == nullptr) {
        authRequest = this->authenticationManager->prepareRequest(nullptr);
      } else {
        authRequest = this->authenticationManager->prepareRequest(&previousResponse->authenticationresponsedata());
      }
      request.set_allocated_authenticationrequestdata(authRequest);
      if (this->authenticationManager->getState() == AuthenticationState::SUCCESS && this->state == State::AUTHENTICATION) {
        this->state = State::KEY_ENTROPY;
      }
      return nullptr;
    }
    if (this->state == State::AUTHENTICATION) {
      throw std::runtime_error("invalid state");
    }
    // send key entropy
    if (this->state == State::KEY_ENTROPY) {
      std::cout << "here prepare request key entropy" << std::endl;
      backup::BackupKeyEntropy *entropy = new backup::BackupKeyEntropy();
      if (this->authenticationManager->getAuthenticationType() == AuthenticationType::PAKE) {
        entropy->set_nonce(randomString());
      } else if (this->authenticationManager->getAuthenticationType() == AuthenticationType::WALLET) {
        entropy->set_rawmessage(randomString());
      }
      request.set_allocated_backupkeyentropy(entropy);
      this->state = State::DATA_HASH;
      return nullptr;
    }
    // send data hash
    if (this->state == State::DATA_HASH) {
      std::cout << "here prepare request data hash" << std::endl;
      request.set_newcompactionhash(randomString());
      this->state = State::CHUNKS;
      return nullptr;
    }
    // send chunks
    if (this->currentChunk >= this->chunkLimit) {
      return std::make_unique<grpc::Status>(grpc::Status::OK);
    }
    std::string dataChunk = randomString(1000);
    std::cout << "here prepare request data chunk " << this->currentChunk << "/" << dataChunk.size() << std::endl;
    request.set_newcompactionchunk(dataChunk);
    ++this->currentChunk;
    return nullptr;
  }
};

class SendLogReactor : public ClientWriteReactorBase<backup::SendLogRequest, google::protobuf::Empty> {
  enum class State {
    AUTHENTICATION = 1,
    LOG_CHUNKS = 2,
  };
  State state = State::AUTHENTICATION;
  const size_t chunkLimit;
  size_t currentChunk = 0;

public:
  SendLogReactor(size_t chunkLimit) : chunkLimit(chunkLimit) {
    std::cout << "send log init with chunks limit: " << chunkLimit << std::endl;
  }

  std::unique_ptr<grpc::Status> prepareRequest(backup::SendLogRequest &request) override {
    switch (this->state) {
      case State::AUTHENTICATION: {
        std::string userID = randomString();
        std::string backupID = randomString();
        backup::SimpleAuthenticationRequestData *authenticationData = new backup::SimpleAuthenticationRequestData();
        authenticationData->set_userid(userID);
        authenticationData->set_backupid(backupID);
        request.set_allocated_authenticationdata(authenticationData);
        this->state = State::LOG_CHUNKS;
        return nullptr;
      }
      case State::LOG_CHUNKS: {
        if (this->currentChunk >= this->chunkLimit) {
          return std::make_unique<grpc::Status>(grpc::Status::OK);
        }
        std::string dataChunk = randomString(1000);
        std::cout << "here prepare request data chunk " << this->currentChunk << "/" << dataChunk.size() << std::endl;
        request.set_logdata(dataChunk);
        ++this->currentChunk;
        return nullptr;
      }
    }
    throw std::runtime_error("invalid state");
  }
};

class Client
{
  std::unique_ptr<backup::BackupService::Stub> stub;

public:
  Client(std::shared_ptr<grpc::Channel> channel);

  std::unique_ptr<CreateNewBackupReactor> createNewBackupReactor;
  std::unique_ptr<SendLogReactor> sendLogReactor;

  void createNewBackup(AuthenticationType authenticationType);
  void sendLog();
  void recoverBackupKey();
  void pullBackup();

  bool reactorActive();
};

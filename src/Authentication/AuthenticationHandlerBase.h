#pragma once

#include "../_generated/backup.pb.h"

#include <google/protobuf/message.h>

#include <memory>
#include <string>

enum class AuthenticationType {
  PAKE = 1,
  WALLET = 2,
};

enum class AuthenticationState {
  IN_PROGRESS = 1,
  SUCCESS = 2,
  FAIL = 3,
};

class AuthenticationHandlerBase {
protected:
  AuthenticationState state = AuthenticationState::IN_PROGRESS;
public:
  virtual backup::FullAuthenticationRequestData *
  prepareRequest(const backup::FullAuthenticationResponseData *previousResponse) = 0;
  virtual AuthenticationType getAuthenticationType() const = 0;
  AuthenticationState getState() {
    return this->state;
  }
  virtual ~AuthenticationHandlerBase() = default;
};

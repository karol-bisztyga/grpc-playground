#pragma once

#include "AuthenticationHandlerBase.h"

#include "../_generated/backup.pb.h"

#include <memory>

class AuthenticationManager {
  std::unique_ptr<AuthenticationHandlerBase> authenticationHandler;
  AuthenticationType authenticationType;

public:
  AuthenticationManager(AuthenticationType authenticationType);

  AuthenticationState getState() const;
  AuthenticationType getAuthenticationType() const;
  backup::FullAuthenticationRequestData *
  prepareRequest(const backup::FullAuthenticationResponseData *previousResponse);
};

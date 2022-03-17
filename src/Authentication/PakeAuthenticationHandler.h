#pragma once

#include "AuthenticationHandlerBase.h"

#include <atomic>
#include <string>

enum class PakeAuthenticationState {
  REGISTRATION = 1,
  REGISTRATION_UPLOAD = 2,
  CREDENTIAL = 3,
  CREDENTIAL_FINALIZATION = 4,
  MAC_EXCHANGE = 5,
};

class PakeAuthenticationHandler : public AuthenticationHandlerBase {
  const AuthenticationType authenticationType = AuthenticationType::PAKE;
  std::atomic<PakeAuthenticationState> pakeState =
      PakeAuthenticationState::REGISTRATION;

public:
  backup::FullAuthenticationRequestData *
  prepareRequest(const backup::FullAuthenticationResponseData *previousResponse) override;
  AuthenticationType getAuthenticationType() const override;
};

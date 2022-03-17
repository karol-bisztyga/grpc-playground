#pragma once

#include "AuthenticationHandlerBase.h"

#include <string>

class WalletAuthenticationHandler : public AuthenticationHandlerBase {
  const AuthenticationType authenticationType = AuthenticationType::WALLET;

public:
  backup::FullAuthenticationRequestData *
  prepareRequest(const backup::FullAuthenticationResponseData *previousResponse) override;
  AuthenticationType getAuthenticationType() const override;
};

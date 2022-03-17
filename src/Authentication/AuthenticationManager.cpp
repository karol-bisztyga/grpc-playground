#include "AuthenticationManager.h"

#include "PakeAuthenticationHandler.h"
#include "WalletAuthenticationHandler.h"

AuthenticationManager::AuthenticationManager(AuthenticationType authenticationType) : authenticationType(authenticationType) {
  if (authenticationType == AuthenticationType::PAKE) {
    this->authenticationHandler =
        std::make_unique<PakeAuthenticationHandler>();
  } else if (authenticationType == AuthenticationType::WALLET) {
    this->authenticationHandler =
        std::make_unique<WalletAuthenticationHandler>();
  }
}

AuthenticationState AuthenticationManager::getState() const {
  if (this->authenticationHandler == nullptr) {
    throw std::runtime_error("trying to get a state of authentication handler that's not been initialized yet");
  }
  return this->authenticationHandler->getState();
}

AuthenticationType AuthenticationManager::getAuthenticationType() const {
  return this->authenticationHandler->getAuthenticationType();
}

backup::FullAuthenticationRequestData *
AuthenticationManager::prepareRequest(const backup::FullAuthenticationResponseData *previousResponse)
{
  return this->authenticationHandler->prepareRequest(previousResponse);
}

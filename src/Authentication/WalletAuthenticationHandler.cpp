#include "WalletAuthenticationHandler.h"

backup::FullAuthenticationRequestData *
WalletAuthenticationHandler::prepareRequest(
    const backup::FullAuthenticationResponseData *previousResponse)
{
  return new backup::FullAuthenticationRequestData();
}

AuthenticationType WalletAuthenticationHandler::getAuthenticationType() const {
  return this->authenticationType;
}

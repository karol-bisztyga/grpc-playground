#include "PakeAuthenticationHandler.h"

#include "Tools.h"

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
SEND_CHUNKS
client => data chunk 0 => server
client => data chunk 1 => server
client => data chunk ... => server
client => data chunk n => server

end connection
*/
backup::FullAuthenticationRequestData *
PakeAuthenticationHandler::prepareRequest(const backup::FullAuthenticationResponseData *previousResponse)
{
  backup::PakeAuthenticationRequestData *requestData = new backup::PakeAuthenticationRequestData();
  const backup::PakeAuthenticationResponseData *responseData = (previousResponse != nullptr) ? &previousResponse->pakeauthenticationresponsedata() : nullptr;

  if (this->getState() != AuthenticationState::IN_PROGRESS) {
    throw std::runtime_error(
        "authentication is terminated but additional action has been "
        "requested");
  }

  // todo in case of any failure you can:
  // - throw here - that terminates the connection
  // - set `this->state = AuthenticationState::FAIL;` and throw in the outter
  // scope
  if (this->pakeState != PakeAuthenticationState::REGISTRATION && responseData == nullptr)
  {
    throw std::runtime_error("response expected but not received");
  }
  switch (this->pakeState) {
    case PakeAuthenticationState::REGISTRATION: {
      std::string userID = std::to_string(randomNumber(1000, 10000));
      std::string registrationData = randomString();
      // todo process with PAKE lib
      // ...
      backup::PakeRegistrationRequestAndUserID *registrationAndUserID = new backup::PakeRegistrationRequestAndUserID();
      registrationAndUserID->set_userid(userID);
      registrationAndUserID->set_pakeregistrationrequest(registrationData);
      requestData->set_allocated_pakeregistrationrequestanduserid(registrationAndUserID);
      this->pakeState = PakeAuthenticationState::REGISTRATION_UPLOAD;
      break;
    }
    case PakeAuthenticationState::REGISTRATION_UPLOAD: {
      std::string registrationResponseData = responseData->pakeregistrationresponse();
      // todo process with PAKE lib
      // ...
      std::string registrationUpload = randomString();
      requestData->set_pakeregistrationupload(registrationUpload);
      this->pakeState = PakeAuthenticationState::CREDENTIAL;
      break;
    }
    case PakeAuthenticationState::CREDENTIAL: {
      bool registrationSuccess = responseData->pakeregistrationsuccess();
      // requestData.pakecredentialrequest();
      // todo process with PAKE lib
      // ...
      std::string credential = randomString();
      requestData->set_pakecredentialrequest(credential);
      this->pakeState = PakeAuthenticationState::CREDENTIAL_FINALIZATION;
      break;
    }
    case PakeAuthenticationState::CREDENTIAL_FINALIZATION: {
      std::string credentialResponse = responseData->pakecredentialresponse();
      //  requestData.pakecredentialfinalization();
      //  todo process with PAKE lib
      //  ...
      //  responseData->set_pakeservermac("...");
      std::string credentialFinalization = randomString();
      requestData->set_pakecredentialfinalization(credentialFinalization);
      this->pakeState = PakeAuthenticationState::MAC_EXCHANGE;
      break;
    }
    case PakeAuthenticationState::MAC_EXCHANGE: {
      std::string pakeServerMAC = responseData->pakeservermac();
      // requestData.pakeclientmac();
      // todo process with PAKE lib
      // ...
      std::string clientMac = randomString();
      requestData->set_pakeclientmac(clientMac);
      // mock a success for now
      this->state = AuthenticationState::SUCCESS;
      break;
    }
  }
  backup::FullAuthenticationRequestData *request =
      new backup::FullAuthenticationRequestData();
  request->set_allocated_pakeauthenticationrequestdata(requestData);
  return request;
}

AuthenticationType PakeAuthenticationHandler::getAuthenticationType() const {
  return this->authenticationType;
}

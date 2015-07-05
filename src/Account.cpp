/**************************************************************************
    This is a general SDK for OpenStack Swift API written in C++
    Copyright (C) <2014>  <Behrooz Shafiee Sarjaz>
    This program comes with ABSOLUTELY NO WARRANTY;

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "Account.h"
#include "Container.h"
#include "HTTPIO.h"
#include <Poco/Exception.h>
#include <sstream>
using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Swift {

/** Initialize Static members **/
uint32_t Account::numOfCalls = 0;

struct Role {
  string name = "null";
  Role() {
  }
  static Role* fromJSON(const Json::Value &value) {
    Role* instance = new Role();
    instance->name = value.get("name", Json::Value::null).asString();
    return instance;
  }

  static Json::Value* toJSON(const Role &instance) {
    Json::Value* root = new Json::Value();
    (*root)["name"] = instance.name;
    return root;
  }
};

Account::~Account() {
  //Delete Token
  delete token;
  token = nullptr;
  //Delete roles
  for(Role* role:roles) {
    delete role;
    role = nullptr;
  }
  roles.clear();
  //Delete Services
  for(Service* service:services) {
    delete service;
    service = nullptr;
  }
  services.clear();

}

Account::Account() :
    userID(""), name(""), token(nullptr), allowReauthenticate(false), preferredRegion(
        ""), delimiter('/') {
  // TODO Auto-generated constructor stub

}

SwiftResult<Account*>* Account::authenticate(
    const AuthenticationInfo &_authInfo, bool _allowReauthenticate) {
  // Create Json Request
  Json::Value jReq;
  Json::Value auth;
  if (_authInfo.tenantName != "")
    auth["tenantName"] = _authInfo.tenantName;
  auth["passwordCredentials"]["username"] = _authInfo.username;
  auth["passwordCredentials"]["password"] = _authInfo.password;
  jReq["auth"] = auth;
  Json::FastWriter writer;

  //Make Network Request
  string req = writer.write(jReq);
  string contentType = "application/json";
  //Create HTTPSession
  HTTPClientSession* httpSession = nullptr;
  HTTPResponse *httpResponse = new HTTPResponse();
  istream* inputStream = nullptr;
  try {
    URI uri(_authInfo.authUrl);
    httpSession = doHTTPIO(uri, HTTPRequest::HTTP_POST, nullptr, req,
        contentType);
    //Receive Data
    inputStream = &httpSession->receiveResponse(*httpResponse);
  } catch (Exception &e) {
    SwiftResult<Account*> *result = new SwiftResult<Account*>();
    SwiftError error(SwiftError::SWIFT_EXCEPTION, e.displayText());
    result->setError(error);
    //Try to set HTTP Response as the payload
    //httpSession->receiveResponse(*httpResponse);
    result->setResponse(httpResponse);
    result->setSession(httpSession);
    result->setPayload(nullptr);
    return result;
  }

  //Check HTTP status
  if (httpResponse->getStatus() != HTTPResponse::HTTPStatus::HTTP_OK) {
    SwiftResult<Account*> *result = new SwiftResult<Account*>();
    SwiftError error(SwiftError::SWIFT_HTTP_ERROR, httpResponse->getReason());
    result->setError(error);
    result->setResponse(httpResponse);
    result->setSession(httpSession);
    result->setPayload(nullptr);
    return result;
  }

  //Everything seems fine
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(*inputStream, root, true);
  if (!parsingSuccessful) {
    SwiftResult<Account*> *result = new SwiftResult<Account*>();
    SwiftError error(SwiftError::SWIFT_JSON_PARSE_ERROR,
        reader.getFormattedErrorMessages());
    result->setError(error);
    result->setResponse(httpResponse);
    result->setSession(httpSession);
    result->setPayload(nullptr);
    return result;
  }
  //Parsing JSON Successful
  //Everything is inside "access"
  root = root["access"];
  Account* instance = new Account();
  //Parse User Info
  Json::Value userRoot = root.get("user", Json::nullValue);
  instance->userID = userRoot.get("id", "").asString();
  instance->name = userRoot.get("name", "").asString();
  instance->authInfo = _authInfo;
  instance->authInfo.username = userRoot.get("username", "").asString();
  //Roles
  Json::Value roles = userRoot.get("roles", Json::nullValue);
  if (roles != Json::nullValue) {
    for (unsigned int i = 0; i < roles.size(); i++) {
      Role *role = Role::fromJSON(roles[i]);
      instance->roles.push_back(role);
    }
  }
  //Parse Token
  Json::Value tokenRoot = root.get("token", Json::nullValue);
  instance->token = Token::fromJSON(tokenRoot);
  //Parse Service Information
  Json::Value serviceRoot = root.get("serviceCatalog", Json::nullValue);
  if (serviceRoot != Json::nullValue)
    for (unsigned int i = 0; i < serviceRoot.size(); i++)
      instance->services.push_back(Service::fromJSON(serviceRoot[i]));

  //Return result
  instance->authInfo.password = _authInfo.password;
  instance->authInfo.authUrl = _authInfo.authUrl;
  instance->allowReauthenticate = _allowReauthenticate;

  SwiftResult<Account*> *result = new SwiftResult<Account*>();
  SwiftError error(SwiftError::SWIFT_OK, "SWIFT_OK");
  result->setError(error);
  result->setResponse(httpResponse);
  result->setSession(httpSession);
  result->setPayload(instance);

  return result;
}

long Account::getBytesUsed() {
  //TODO implement
  return -1;
}

int Account::getObjectCount() {
  //TODO implement
  return -1;
}

long Account::getServerTime() {
  //TODO implement
  return -1;
}

void Account::setAllowReauthenticate(bool _allowReauthenticate) {
  this->allowReauthenticate = _allowReauthenticate;
}

bool Account::isAllowReauthenticate() {
  return this->allowReauthenticate;
}

uint32_t Account::increaseCallCounter() {
  return ++numOfCalls;
}

uint32_t Account::getNumberOfCalls() {
  return Account::numOfCalls;
}

Tenant* Account::getTenant() {
  return this->token->getTenant();
}

Token* Account::getToken() {
  return this->token;
}

Service* Account::getSwiftService() {
  for (unsigned int i = 0; i < services.size(); i++)
    if (services[i]->getType() == "object-store")
      return services[i];
  return nullptr;
}

string Account::toString() {
  ostringstream output;
  ostringstream roleStream;
  ostringstream serviceStream;
  Json::StyledWriter jsonWriter;

  //Build Roles
  roleStream << "Roles: {";
  for (unsigned int i = 0; i < roles.size(); i++) {
    Json::Value* roleJSON = Role::toJSON(*roles[i]);
    roleStream << jsonWriter.write(*roleJSON) << ",";
    delete roleJSON;
    roleJSON = nullptr;
  }
  roleStream << "}";
  //Build Services
  serviceStream << "Services: {";
  for (unsigned int i = 0; i < services.size(); i++) {
    Json::Value* serviceJSON = Service::toJSON(*services[i]);
    serviceStream << jsonWriter.write(*serviceJSON) << ",";
    delete serviceJSON;
    serviceJSON = nullptr;
  }
  serviceStream << "}";

  Json::Value* tokenJSON = Token::toJSON(*this->token);

  output << "userID:" << userID << ",\n" << "name:" << name << ",\n"
      << "username:" << authInfo.username << ",\n" << "password:"
      << authInfo.password << ",\n" << "authUrl:" << authInfo.authUrl << ",\n"
      << "allowReauthenticate:" << allowReauthenticate << ",\n"
      << "preferredRegion:" << preferredRegion << ",\n" << "delimiter:"
      << delimiter << ",\n" << "authenticationmethod:"
      << authenticationMethodToString(authInfo.method) << ",\n" << "token:"
      << jsonWriter.write(*tokenJSON) << ",\n"
      << roleStream.str() << ",\n" << serviceStream.str();

  delete tokenJSON;
  tokenJSON = nullptr;
  return output.str();
}

SwiftResult<istream*>* Account::swiftAccountDetails(HTTPHeader &_formatHeader,
    vector<HTTPHeader> *_reqMap, bool _newest)
{
  /**
   * Check HTTP return code
   * 200:
   *  Success. The response body lists the containers.
   *
   * 204:
   *  Success. The response body shows no containers.
   *  Either the account has no containers or you are paging
   *  through a long list of names by using the marker, limit,
   *  or end_marker query parameters, and you have reached
   *  the end of the list.
   */
  vector<int> validHTTPCodes;
  validHTTPCodes.push_back(HTTPResponse::HTTP_OK);
  validHTTPCodes.push_back(HTTPResponse::HTTP_NO_CONTENT);

  //add X-Newest
  bool shouldDelete = false;
  if (_newest) {
    if(_reqMap == nullptr) {
      _reqMap = new vector<HTTPHeader>();
      shouldDelete = true;
    }
    _reqMap->push_back(*new HTTPHeader("X-Newest", "True"));
  }

  vector<HTTPHeader> uriParams;
  uriParams.push_back(_formatHeader);

  //Do swift transaction
  string path = "";
  SwiftResult<istream*> *result = doSwiftTransaction<istream*>(this, path,
          HTTPRequest::HTTP_GET, &uriParams, _reqMap, &validHTTPCodes, nullptr, 0,
            nullptr);
  if(!shouldDelete)
    return result;
  else {
    delete _reqMap;
    return result;
  }

}

SwiftResult<int*>* Account::swiftCreateMetadata(
    vector<pair<string, string> >& _metaData, vector<HTTPHeader>* _reqMap) {
  /**
   * 204:
   *  Success. The response body is empty.
   */
  vector<int> validHTTPCodes;
  validHTTPCodes.push_back(HTTPResponse::HTTP_NO_CONTENT);

  //Add Actual metadata
  bool shouldDelete = false;
  if (_metaData.size() > 0) {
    if (_reqMap == nullptr) {
      _reqMap = new vector<HTTPHeader>();
      shouldDelete = true;
    }
    for (unsigned int i = 0; i < _metaData.size(); i++) {
      HTTPHeader header("X-Account-Meta-" + _metaData[i].first, _metaData[i].second);
      _reqMap->push_back(header);
    }
  }

  //Do swift transaction
  string path = "";
  SwiftResult<int*>* result = doSwiftTransaction<int*>(this, path,
      HTTPRequest::HTTP_POST, nullptr, _reqMap, &validHTTPCodes, nullptr, 0,
        nullptr);
  if(!shouldDelete)
    return result;
  else {
    delete _reqMap;
    return result;
  }
}

SwiftResult<int*>* Account::swiftUpdateMetadata(
    vector<pair<string, string> >& _metaData, vector<HTTPHeader>* _reqMap) {
  return swiftCreateMetadata(_metaData, _reqMap);
}

SwiftResult<int*>* Account::swiftDeleteMetadata(
    std::vector<string> &_metaDataKeys, vector<HTTPHeader>* _reqMap) {
  /**
   * 204:
   *  Success. The response body is empty.
   */
  vector<int> validHTTPCodes;
  validHTTPCodes.push_back(HTTPResponse::HTTP_NO_CONTENT);

  //Add Actual metadata
  bool shouldDelete = false;
  if (_metaDataKeys.size() > 0) {
    if (_reqMap == nullptr) {
      _reqMap = new vector<HTTPHeader>();
      shouldDelete = true;
    }
    for (unsigned int i = 0; i < _metaDataKeys.size(); i++) {
      HTTPHeader header("X-Remove-Account-Meta-" + _metaDataKeys[i], "x");
      _reqMap->push_back(header);
    }
  }

  //Do swift transaction
  string path = "";
  SwiftResult<int*>* result = doSwiftTransaction<int*>(this, path,
      HTTPRequest::HTTP_POST, nullptr, _reqMap, &validHTTPCodes, nullptr, 0,
        nullptr);
  if(!shouldDelete)
      return result;
    else {
      delete _reqMap;
      return result;
    }
}

SwiftResult<vector<Container>*>* Account::swiftGetContainers(bool _newest) {
  SwiftResult<istream*> *accountDetail = this->swiftAccountDetails(HEADER_FORMAT_APPLICATION_JSON,nullptr,_newest);
  SwiftResult<vector<Container>*> *result = new SwiftResult<vector<Container>*>();
  result->setError(accountDetail->getError());
  result->setResponse(nullptr);
  result->setSession(nullptr);

  //Check error
  if(accountDetail->getError().code != SWIFT_OK.code) {
    result->setPayload(nullptr);
    delete accountDetail;
    return result;
  }

  //Parse JSON
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(*accountDetail->getPayload(), root, false);
  if (!parsingSuccessful) {
    SwiftError error(SwiftError::SWIFT_JSON_PARSE_ERROR,
        reader.getFormattedErrorMessages());
    result->setError(error);
    result->setPayload(nullptr);
    delete accountDetail;
    return result;
  }

  //Allocate containers
  vector<Container>*containers = new vector<Container>();
  //Successful parse
  for(unsigned int i=0;i<root.size();i++) {
    string name = root[i].get("name","").asString();
    string count = root[i].get("count","").asString();
    string bytes = root[i].get("bytes","").asString();

    Container container(this,name);
    container.setBytesUsed(stoul(bytes));
    container.setTotalObjects(stoul(count));

    containers->push_back(container);
  }

  //Set payload
  result->setPayload(containers);
  delete accountDetail;
  return result;
}

SwiftResult<int*>* Account::swiftShowMetadata(bool _newest) {
  /**
   * 204:
   *  Success. The response body is empty.
   */
  vector<int> validHTTPCodes;
  validHTTPCodes.push_back(HTTPResponse::HTTP_NO_CONTENT);

  vector<HTTPHeader> _reqMap;
  if (_newest)
    _reqMap.push_back(*new HTTPHeader("X-Newest", "True"));

  //Do swift transaction
  string path = "";
  return doSwiftTransaction<int*>(this, path,
      HTTPRequest::HTTP_HEAD, nullptr, &_reqMap, &validHTTPCodes, nullptr, 0,
        nullptr);
}

} /* namespace Swift */

bool Swift::Account::reAuthenticate() {
  //Use authenticate function
  SwiftResult<Account*> *tempAccount = authenticate(authInfo,true);

  //Check error
  if(tempAccount->getError().code != SWIFT_OK.code) {
    delete tempAccount;
    return false;
  }

  //Parse User Info
  this->userID = tempAccount->getPayload()->userID;
  //Parse Token
  *this->token = *tempAccount->getPayload()->token;

  delete tempAccount;
  return true;
}

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

#ifndef ACCOUNT_H_
#define ACCOUNT_H_

#include "Token.h"
#include "Service.h"
#include "Authentication.h"
#include "SwiftResult.h"
#include "Header.h"

#include <cstdint>
#include <iostream>
#include <vector>

namespace Swift {

/** Associated roles with this account **/
struct Role;

/** Forward declare Container **/
class Container;

class Account {
private:

  /**
   * Account user ID
   */
  std::string userID;

  /**
   * name
   */
  std::string name;

  /**
   * ObjectStore token
   */
  Token *token;

  /**
   * Authentication Information
   */
  AuthenticationInfo authInfo;

  /**
   * Account Roles
   */
  std::vector<Role*> roles;

  /**
   * Openstack Services
   */
  std::vector<Service*> services;

  /**
   * ObjectStore tokens expire after 24 hours. If reauthentication is allowed (default behaviour),
   * after expiration, Account will reauthenticate itself and get a new authentication token. If
   * reauthentication is turned off, you will have to manually arrange reauthentication. This mode
   * is recommended for web servers or otherwise long-living processes.
   */
  bool allowReauthenticate = true;

  /**
   * The preferred region will determine what Swift end-point will be chosen. If no preferred region is
   * set (default), the first applicable end-point will be selected.
   */
  std::string preferredRegion;

  /**
   * The delimiter is used to check for directory boundaries. The default will be a '/'.
   */
  char delimiter = '/';

  /**
   * Number of calls made to Object Store
   */
  static uint32_t numOfCalls;

public:
  virtual ~Account();
  Account();

  /**
   * Trigger the authentication against Object Store. There are two use cases for this method. The first is
   * triggered pro-actively by the user by calling authenticate on the client. The second is when the token
   * has expired and AbstractSecureCommand triggers a re-authentication.
   * @return the access element including a new token
   */
  static SwiftResult<Account*>* authenticate(
      const AuthenticationInfo &_authInfo, bool _allowReauthenticate = true);

  bool reAuthenticate();

  /**
   * The number of bytes stored by the StoredObjects in all Containers in the Account.
   * @return number of bytes
   */
  long getBytesUsed();

  /**
   * The number of StoredObjects in all Containers in the Account.
   * @return number of StoredObjects
   */
  int getObjectCount();

  /**
   * Returns the server time in milliseconds since 1970
   * @return server time
   */
  long getServerTime();

  void setAllowReauthenticate(bool _allowReauthenticate);

  /**
   * Whether reauthentication on expiration of the authentication token is allowed, or this is done
   * manually by the client
   * @return whether reauthentication is allowed
   */
  bool isAllowReauthenticate();

  /**
   * Increase the call counter which tracks how many calls are made to the ObjectStore
   */
  static uint32_t increaseCallCounter();

  /**
   * Returns the number of HTTP calls that have been made to the ObjectStore server. This could be
   * useful to check the efficiency of the methods and configuration in use
   * @return number of calls made to the ObjectStore server
   */
  static uint32_t getNumberOfCalls();

  /**
   * Returns the tenant of the account.
   * @return the tenant belonging to the Account
   */
  Tenant* getTenant();

  /**
   * Returns the tenant of the account.
   * @return the tenant belonging to the Account
   */
  Token* getToken();

  /**
   * String representation of this class
   * @return string containing all the objects of this account
   */
  std::string toString();

  /**
   * Get Object Storage service
   * @return swift service
   */
  Service* getSwiftService();

  /** API Functions **/

  /**
   * Returns all the containers under this account
   * @return
   *  a vector of Containers
   * _newest
   *  If set to True, Object Storage queries all replicas
   *  to return the most recent one. If you omit this header,
   *  Object Storage responds faster after it finds one valid
   *  replica. Because setting this header to True is more
   *  expensive for the back end, use it only when it is
   *  absolutely needed.
   */
  SwiftResult<std::vector<Container>*>* swiftGetContainers(bool _newest = false);

  /**
   * Shows details for this account
   * @return
   *  An stream containing details of this account
   * _formatHeader
   *  Specifies format of returned information
   * _reqMap
   *  You can add additional query parameters to this request. Please refer
   *  the Swift API documentations to see available query parameters.
   *  http://docs.openstack.org/api/openstack-object-storage/1.0/content/index.html
   */
  SwiftResult<std::istream*>* swiftAccountDetails(HTTPHeader &_formatHeader =
      HEADER_FORMAT_APPLICATION_JSON,
      std::vector<HTTPHeader> *_reqMap = nullptr, bool _newest = false);

  /**
   * Adds metadata to this account
   * @return
   *  Nothing
   * _metaData
   *  A vector of string pairs (key,value)
   */
  SwiftResult<int*>* swiftCreateMetadata(
      std::vector<std::pair<std::string, std::string>> &_metaData,
      std::vector<HTTPHeader> *_reqMap = nullptr);

  /**
   * Updates existing metadata for this account
   * @return
   *  Nothing
   * _metaData
   *  A vector of string pairs (key,value)
   */
  SwiftResult<int*>* swiftUpdateMetadata(
      std::vector<std::pair<std::string, std::string>> &_metaData,
      std::vector<HTTPHeader> *_reqMap = nullptr);

  /**
   * Removes specified metadata (with key) from this account
   * @return
   *  Nothing
   * _metaDataKeys
   *  A vector containing keys of metadata which should be removed.
   */
  SwiftResult<int*>* swiftDeleteMetadata(
      std::vector<std::string> &_metaDataKeys,
      std::vector<HTTPHeader> *_reqMap = nullptr);

  /**
   * Gets the existing metadata for this account
   * @return
   *  Nothing. The payload is nullptr; however, the returned metadata are
   *  part of httpresponse. For example, getResponse()->write(cout);
   */
  SwiftResult<int*>* swiftShowMetadata(bool _newest = false);
};

} /* namespace Swift */
#endif /* ACCOUNT_H_ */

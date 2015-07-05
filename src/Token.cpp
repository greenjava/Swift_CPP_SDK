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

#include "Token.h"

namespace Swift {

Token::Token() :
    expires(""), id(""), issued_at(""), tenant(nullptr) {
    // TODO Auto-generated constructor stub

}

Token::Token(const std::string& _expires, const std::string& _id,
        const std::string& _issued_at, Tenant* _tenant) {
    this->expires = _expires;
    this->id = _id;
    this->issued_at = _issued_at;
    this->tenant = _tenant;
}

Token::~Token() {
    delete tenant;
    tenant = nullptr;
}

Token* Token::fromJSON(const Json::Value &val) {
    Token *instance = new Token();
    instance->setExpires(val.get("expires", Json::Value::null).asString());
    instance->setId(val.get("id", Json::Value::null).asString());
    instance->setIssuedAt(val.get("issued_at", Json::Value::null).asString());
    Json::Value tenant = val.get("tenant", Json::Value::null);
    instance->setTenant(Tenant::fromJSON(tenant));

    return instance;
}

Json::Value* Token::toJSON(const Token &instance) {
    Json::Value* json = new Json::Value();

    Json::Value* tenantJSON = Tenant::toJSON(*instance.getTenant());

    (*json)["id"] = instance.getId();
    (*json)["expires"] = instance.getExpires();
    (*json)["issued_at"] = instance.getIssuedAt();
    (*json)["tenant"] = *tenantJSON;

    delete tenantJSON;
    tenantJSON = nullptr;
    return json;
}

const std::string& Token::getExpires() const {
    return expires;
}

void Token::setExpires(const std::string& expires) {
    this->expires = expires;
}

const std::string& Token::getId() const {
    return id;
}

void Token::setId(const std::string& id) {
    this->id = id;
}

const std::string& Token::getIssuedAt() const {
    return issued_at;
}

void Token::setIssuedAt(const std::string& issuedAt) {
    issued_at = issuedAt;
}

Tenant* Token::getTenant() const {
    return tenant;
}

void Token::setTenant(Tenant* tenant) {
    this->tenant = tenant;
}

Token& Token::operator =(const Token& other) {
  expires = other.expires;
  id = other.id;
  issued_at = other.issued_at;
  if(tenant!=nullptr && other.tenant!=nullptr) {
    tenant->setDescription(other.tenant->getDescription());
    tenant->setId(other.tenant->getId());
    tenant->setName(other.tenant->getName());
    tenant->setEnabled(other.tenant->isEnabled());
  }
  else
    tenant = other.tenant;
  return *this;
}

} /* namespace Swift */

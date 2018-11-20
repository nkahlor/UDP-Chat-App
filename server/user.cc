//
// Created by Nicholas Kahlor on 11/8/18.
//

#include "user.h"

User::User(std::string user, std::string pass, std::string tok) {
    this->username = user;
    this->password = pass;
    this->token = tok;
}

const std::string &User::getUsername() const {
    return username;
}

const std::string &User::getPassword() const {
    return password;
}

const std::string &User::getToken() const {
    return token;
}
const std::string &User::getIp() const {
    return ip;
}
const std::string &User::getPort() const {
    return port;
}

const struct sockaddr_in &User::getAddress() const {
    return address;
}

void User::setToken(const std::string &token) {
    User::token = token;
}

void User::setIp(const std::string &ip) {
    User::ip = ip;
}

void User::setPort(const std::string &port) {
    User::port = port;
}

void User::setAddress(const struct sockaddr_in addr) {
    User::address = addr;
}
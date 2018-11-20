//
// Created by Nicholas Kahlor on 11/8/18.
//

#ifndef CLIENT_SERVER_CHAT_CLIENT_H
#define CLIENT_SERVER_CHAT_CLIENT_H
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

class User {
private:
    std::string username;
    std::string password;
    std::string token;
    std::string ip;
    std::string port;
    time_t last_message_time;
    struct sockaddr_in address;

public:
    User() {}
    User(std::string, std::string, std::string);
    const std::string &getUsername() const;
    const std::string &getPassword() const;
    const std::string &getToken() const;
    const std::string &getIp() const;
    const std::string &getPort() const;
    const time_t &getLastMesgTime() const;
    const struct sockaddr_in &getAddress() const;
    void setToken(const std::string &token);
    void setIp(const std::string &ip);
    void setPort(const std::string &port);
    void setAddress(const struct sockaddr_in addr);
    void renewSession();
};


#endif //CLIENT_SERVER_CHAT_CLIENT_H

//
// Created by Nicholas Kahlor on 11/9/18.
//

#ifndef CLIENT_SERVER_CHAT_PORTAL_H
#define CLIENT_SERVER_CHAT_PORTAL_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MESSAGE_SIZE 1024 // max of 1KB messages

class Portal {
protected:
    struct sockaddr_in host_addr, remote_addr;
    int sockfd;
    socklen_t len;
    bool connected;

public:
    Portal();
    std::string receiveMessageRaw();
    bool sendMessageRaw(std::string mesg);
    bool validPreamble(std::string mesg);   // Preamble is username->server# or server->username# (username is any string)
    void close();
    bool isConnected();
};

#endif //CLIENT_SERVER_CHAT_PORTAL_H

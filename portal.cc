//
// Created by Nicholas Kahlor on 11/9/18.
//
#include "portal.h"

Portal::Portal() {
    srand((unsigned int)time(nullptr));
}

std::string Portal::receiveMessageRaw() {
    char buf[MESSAGE_SIZE];
    ssize_t recv_len;
    len = sizeof(remote_addr);
    recv_len = recvfrom(sockfd,                            // socket file descriptor
                        buf,                               // receive buffer
                        sizeof(buf),                       // number of bytes to be received
                        0,
                        (struct sockaddr *) &remote_addr,  // client address
                        &len);                             // length of client address structure

    if (recv_len <= 0) {
       // printf("recvfrom() error: %s.\n", strerror(errno));
        return "";
    }

    std::string message(buf);
    std::cout << "Received: " << message << std::endl;
    return message;
}

bool Portal::sendMessageRaw(std::string message) {
    ssize_t sent_len;
    char buf[MESSAGE_SIZE];
    std::cout << "Sending: " << message << std::endl;
    for(unsigned long i = 0; i < message.size(); i++)
        buf[i] = message.at(i);
    for(unsigned long i = message.size(); i < MESSAGE_SIZE; i++)
        buf[i] = '\0';

    sent_len = sendto(sockfd,                           // the socket file descriptor
                      buf,                              // the sending buffer
                      MESSAGE_SIZE,                      // the number of bytes you want to send
                      0,
                      (struct sockaddr *) &remote_addr, // destination address
                      sizeof(remote_addr));             // size of the address

    if(sent_len < sizeof(buf)) {
        printf("sendto() error: %s.\n", strerror(errno));
        return false;
    }
    return true;
}

bool Portal::validPreamble(std::string mesg) {

    return true;
}

void Portal::close() {
    ::close(sockfd);
    connected = false;
}

bool Portal::isConnected() {
    return connected;
}

char Portal::_generate_char() {
    auto random_char = (unsigned char)(rand() % (126 - 33 + 1) + 33);
    return static_cast<char>(random_char);
}

std::string Portal::_generate_msg_id() {
    const int ID_LEN = 10;
    std::string id;
    for(int i = 0; i < ID_LEN; i++)
        id += _generate_char();
    return id;
}
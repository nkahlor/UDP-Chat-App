#include <iostream>
#include <pthread.h>

#include "udp_client_portal.h"

ClientPortal client;

pthread_t receiver;
void *receive_messages(void*);

// Exit gracefully
void sighandler(int sig_num)
{
    client.close();
    exit(0);
}

int main() {
    std::string message;
    std::string user, pass, ip, port;

    // Attempt to log the user into the server
    std::cout << "Enter your username\n>> ";
    std::cin >> user;
    std::cout << "Enter your password\n>> ";
    std::cin >> pass;
    std::cout << "Enter your IP\n>> ";
    std::cin >> ip;
    std::cout << "Enter your Port\n>> ";
    std::cin >> port;
    client.loginToServer(user, pass , ip, port);

    // Continually accept messages from the server
    pthread_create(&receiver, nullptr, receive_messages, nullptr);

    signal(SIGTSTP, sighandler);
    while(true) {
        std::string dest;
        std::cout << "Who do you want to message?\n>> ";
        std::cin >> dest;
        std::cout << "Enter the message body: \n>> ";
        // Eat the newline
        std::getline(std::cin, message);
        // Get the actual message
        std::getline(std::cin, message);
        client.sendMessage(message, dest);
    }

    return 0;
}

void *receive_messages(void*) {
    while(true) {
        std::cout << client.receiveMessageRaw() << std::endl;
    }
}
#ifndef CLIENT_SERVER_CHAT_CLIENT_PORTAL_H
#define CLIENT_SERVER_CHAT_CLIENT_PORTAL_H

#include <iostream>

#include "../portal.h"

class ClientPortal : public Portal {
private:
    std::string token;
    std::string user;
    std::string pass;

    static ClientPortal* __instance;

    std::string _generate_msg_id();
    char _generate_char();
public:
    ClientPortal();
    static ClientPortal* getInstance();
    bool loginToServer(std::string user, std::string pass, std::string ip, std::string port);
    void sendMessage(std::string content, std::string dest);
    void logoutOfServer();
};

#endif

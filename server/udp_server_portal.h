#ifndef CLIENT_SERVER_CHAT_SERVER_PORTAL_H
#define CLIENT_SERVER_CHAT_SERVER_PORTAL_H

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <pthread.h>

#include "user.h"
#include "../portal.h"

#define TIMEOUT_LEN 300 // seconds

class ServerPortal : public Portal {
private:
    enum CMD { LOGIN = 0, LOGOFF, MESSAGE };
    const std::string INVALID_FORMAT_ERR = "Error: unrecognized message format";

    std::vector<User> users;
    std::map<std::string, User> logged_in;
    std::ofstream logfile;  // maintains every message sent

    // Log inactive users out every 5 minutes periodically
    pthread_t timeout;

    CMD _get_cmd(std::string mesg);
    bool _log_user_in(std::string mesg);
    bool _log_user_out(std::string mesg);;
    std::string _generate_token();
    bool _valid_token(std::string tok);
    bool _route_message(std::string mesg);
    bool _forward_message_to_usr(std::string mesg, User src, User dest);
    bool _report_to_sender(std::string mesg, User sender);

public:
    ServerPortal();
    void parseMessage(const std::string mesg);
    bool validUser();
};

#endif
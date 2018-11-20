#ifndef CLIENT_SERVER_CHAT_SERVER_PORTAL_H
#define CLIENT_SERVER_CHAT_SERVER_PORTAL_H

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "user.h"
#include "../portal.h"

class ServerPortal : public Portal {
private:
    enum CMD { LOGIN = 0, LOGOFF, MESSAGE };
    const std::string INVALID_FORMAT_ERR = "Error: unrecognized message format";

    std::vector<User> users;
    std::map<std::string, User> logged_in;
    std::ofstream logfile;  // maintains every message sent

    CMD _get_cmd(std::string mesg);
    bool _log_user_in(std::string mesg);
    bool _log_user_out(std::string mesg);;
    std::string _generate_token();
    bool _valid_token(std::string tok);
    char _generate_char();
    std::string _generate_msg_id();
    bool _route_message(std::string mesg);
    bool _send_message_to_usr(std::string mesg, User usr);

public:
    ServerPortal();
    void parseMessage(const std::string mesg);
    bool validUser();
};

#endif
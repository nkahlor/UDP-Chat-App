#include "udp_server_portal.h"

// Timeout thread task body
void* __disconnect_inactive_users(void*);

ServerPortal::ServerPortal() : Portal() {
    // Open the Server's connection
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        connected = false;
    }
    else {
        // The host_addr is the address and port number that the server will
        // keep receiving from.
        memset(&host_addr, 0, sizeof(host_addr));
        host_addr.sin_family = AF_INET;
        host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        host_addr.sin_port = htons(32000);

        struct timeval tv;
        tv.tv_sec = 5; // Wait no more than 5s for any single message
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        bind(sockfd,
             (struct sockaddr *) &host_addr,
             sizeof(host_addr));
        connected = true;
    }

    // Read in all valid users
    std::ifstream in("server/users.txt");
    std::string line;
    while(std::getline(in, line) && in.is_open()) {
        size_t split_at = line.find_first_of(':');
        User newUser = User(line.substr(0, split_at), line.substr(split_at + 1, line.size() - 1), "");
        users.push_back(newUser);
    }

    // Initialize the output file to store messages
    logfile = std::ofstream("server/archive.log");

    // Initialize timeout thread
    pthread_create(&timeout, nullptr, __disconnect_inactive_users, &logged_in);
}

void ServerPortal::parseMessage(const std::string mesg) {
    if(validPreamble(mesg)) {
        CMD command = _get_cmd(mesg);
        switch(command) {
            case LOGIN:
                _log_user_in(mesg);
                break;
            case LOGOFF:
                _log_user_out(mesg);
                break;
            case MESSAGE:
                _route_message(mesg);
                break;
        }
    } else {
        sendMessageRaw(INVALID_FORMAT_ERR);
    }
}

ServerPortal::CMD ServerPortal::_get_cmd(std::string mesg) {
    unsigned long preamble_size = mesg.find_first_of('#');
    std::string raw_mesg = mesg.substr(preamble_size + 1, mesg.size() - 1);
    if(raw_mesg.find("login") != std::string::npos)
        return LOGIN;
    else if(raw_mesg.find("logoff") != std::string::npos)
        return LOGOFF;
    else
        return MESSAGE;
}

bool ServerPortal::_log_user_in(std::string mesg) {
    // username->server#login<password><ip><port>
    size_t begin_user = 0;
    size_t end_user = mesg.find_first_of('-');
    size_t begin_pass = mesg.find_first_of('<', end_user + 1) + 1;
    size_t end_pass = mesg.find_first_of('>', begin_pass);
    size_t begin_ip = mesg.find_first_of('<', end_pass + 1) + 1;
    size_t end_ip = mesg.find_first_of('>', begin_ip);
    size_t begin_port = mesg.find_first_of(('<'), end_ip) + 1;
    size_t end_port = mesg.size() - 1;

    // Parse the input to receive the login information
    std::string username = mesg.substr(begin_user, end_user - begin_user);
    std::string password = mesg.substr(begin_pass, end_pass - begin_pass);
    std::string ip = mesg.substr(begin_ip, end_ip - begin_ip);
    std::string port = mesg.substr(begin_port, end_port - begin_port);

    if(logged_in.count(username) > 0) {
        sendMessageRaw("Error: already logged in");
        return false;
    }

    bool validUser = false;
    for(int i = 0; i < users.size(); i++) {
        if(users[i].getUsername() == username) {
            if(users[i].getPassword() == password) {
                validUser = true;
                users[i].setIp(ip);
                users[i].setPort(port);
                users[i].setToken(_generate_token());
                users[i].setAddress(remote_addr); // Current remote address is whoever sent the login request
                users[i].renewSession();
                logged_in[username] = users[i];
                sendMessageRaw(users[i].getToken());
                break;
            }
            else {
                sendMessageRaw("Error: password doesn't match");
                return false;
            }
        }
    }
    if(!validUser) {
        sendMessageRaw("Error: Unregistered username");
    }
    // Broadcast the presence of a new user to all currently logged in users
    else {
        std::string msg_id = _generate_msg_id();
        // Back up the address of the person who logged in
        struct sockaddr_in tmp = remote_addr;
        for(auto const& itr : logged_in) {
            // If the the iterator points to a user besides the one who just logged on, notify them of a new user
            if(itr.second.getUsername() != username) {
                std::string broadcast_mesg = "server->" + itr.second.getUsername() + "#" + "NewUser: " + username;
                remote_addr = itr.second.getAddress();
                sendMessageRaw(broadcast_mesg);
            }
            // Let the new guy know who's already logged in
            if(itr.second.getUsername() != username) {
                std::string broadcast_mesg = "server->" + username + "#" + "NewUser: " + itr.second.getUsername();
                remote_addr = tmp; // Send to the person who logged in
                sendMessageRaw(broadcast_mesg);
            }
        }
        // Restore the address
        remote_addr = tmp;
    }

    return validUser;
}

bool ServerPortal::_log_user_out(std::string mesg) {
    // username->server#logoff
    size_t begin_user = 0;
    size_t end_user = mesg.find_first_of('-');

    std::string username = mesg.substr(begin_user, end_user - begin_user);
    if(logged_in.count(username) > 0) {
        logged_in.erase(username);
        sendMessageRaw("Logged out");
        return true;
    } else {
        sendMessageRaw("Error: Cannot log off when you were never logged in");
        return false;
    }
}

bool ServerPortal::_route_message(std::string mesg) {
    // Break up the message into the proper components
    size_t begin_sender = 0;
    size_t end_sender = mesg.find_first_of('-');
    size_t begin_dest = end_sender + 2;
    size_t end_dest = mesg.find_first_of('#', begin_dest);
    size_t begin_tok = mesg.find_first_of('<', end_dest) + 1;
    size_t end_tok = mesg.find_first_of('>', begin_tok);
    size_t begin_msg_id = mesg.find_first_of('<', end_tok) + 1;
    size_t end_msg_id = mesg.find_first_of('>', begin_msg_id);
    size_t begin_msg = end_msg_id + 1;
    size_t end_msg = mesg.size();
    std::string sender;
    std::string dest;
    std::string tok;
    std::string msg_id;
    std::string message;
    try {
        sender = mesg.substr(begin_sender, end_sender - begin_sender);
        dest = mesg.substr(begin_dest, end_dest - begin_dest);
        tok = mesg.substr(begin_tok, end_tok - begin_tok);
        msg_id = mesg.substr(begin_msg_id, end_msg_id - begin_msg_id);
        message = mesg.substr(begin_msg, end_msg - begin_msg);
    } catch(const std::out_of_range& e) {
        //std::cerr << "Out of range error: " << e.what() << std::endl;
        //sendMessageRaw("Error: Incorrect message format");
        return false;
    }

    if(logged_in.count(sender) > 0) {
        logged_in[sender].renewSession();
    }

    // Verify the sender's Token
    if(!_valid_token(tok)) {
        _report_to_sender("Error: Could not authenticate sender, message not sent", logged_in[sender]);
        return false;
    }

    // Find the IP and Port of the destination
    std::string ip;
    std::string port;
    bool valid_dest = logged_in.count(dest) > 0;
    if(!valid_dest) {
        _report_to_sender("Error: Sending failed, user not logged in", logged_in[sender]);
        return false;
    }

    std::string full_mesg = sender + "->" + dest + "#<" + logged_in[dest].getToken() + ">"  +
            "<" + _generate_msg_id() + ">" +
            sender + ": " + message;
    // Send the message if the user is logged in
    if(!_forward_message_to_usr(full_mesg, logged_in[sender], logged_in[dest])) {
        _report_to_sender("Error: Message failed to send", logged_in[sender]);
        return false;
    }
    // Let the sender know it was successfully sent
    std::string echo = "Success: " + message;

    remote_addr = logged_in[sender].getAddress();
    _report_to_sender(echo, logged_in[sender]);
    // Store the message and messageID
    logfile << "[" << msg_id << "]" << sender << "->" << dest << ": " << message << "\n";
    logfile.flush();
    return true;
}

bool ServerPortal::_report_to_sender(std::string mesg, User sender) {
    std::string full_mesg = "server->" + sender.getUsername() + "#<" + logged_in[sender.getUsername()].getToken() +
                            ">" + "<" + _generate_msg_id() + ">" + mesg;
    sendMessageRaw(full_mesg);
}

bool ServerPortal::_forward_message_to_usr(std::string mesg, User src, User dest) {
    std::string full_mesg = src.getUsername() + "->" + dest.getUsername() + "#<" + logged_in[dest.getUsername()].getToken() +
                            ">" + "<" + _generate_msg_id()
                            + ">" + mesg;
    struct sockaddr_in sender_addr = remote_addr;
    // Set the remote_addr
    remote_addr = dest.getAddress();
    // Send the message itself
    bool success = sendMessageRaw(mesg);
    // Restore the previous sender's address to echo messages properly
    remote_addr = sender_addr;
    return success;
}

// TODO: Ensure no duplicate tokens in the program are created, it's not likely but it is possible
std::string ServerPortal::_generate_token() {
    const int TOKEN_LEN = 6;
    std::string token;
    for(int i = 0; i < TOKEN_LEN; i++)
        token += _generate_char();
    return token;
}

bool ServerPortal::_valid_token(std::string tok) {
    for(const User& u : users) {
        if(u.getToken().find(tok) != std::string::npos) // if the token is found in any user, return true
            return true;
    }
    // if we never found the token, it must be invalid
    return false;
}

// TODO: Send a message to the user you're booting
void* __disconnect_inactive_users(void* data) {
    std::map<std::string, User>* logged_in = (std::map<std::string, User>*) data;
    while(true) {
        sleep(TIMEOUT_LEN); // Periodically awake every 5 minutes
        time_t current_time = time(nullptr);
        for(auto itr = logged_in->begin(); itr != logged_in->end(); itr++) {
            if(current_time - itr->second.getLastMesgTime() >= TIMEOUT_LEN) {
                std::cout << itr->second.getUsername() + " logged out due to inactivity\n";

                logged_in->erase(itr->second.getUsername());
                // Don't try to increment the iterator if this operation emptied the map
                if(logged_in->empty())
                    break;
            }
        }
    }
}
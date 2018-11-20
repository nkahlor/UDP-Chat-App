#include "udp_server_portal.h"

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
                logged_in[username] = users[i];
                sendMessageRaw(users[i].getToken());
                break;
            }
            else {
                sendMessageRaw("Error: password doesn't match");
            }
            break;
        }
    }
    if(!validUser)
        sendMessageRaw("Error: Unregistered username");
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
        std::cerr << "Out of range error: " << e.what() << std::endl;
        sendMessageRaw("Error: Incorrect message format");
        return false;
    }

    // Verify the sender's Token
    if(!_valid_token(tok)) {
        sendMessageRaw("Error: Could not authenticate sender, message not sent");
        return false;
    }

    // Find the IP and Port of the destination
    std::string ip;
    std::string port;
    bool valid_dest = false;
    for(const User& u : users) {
        if(u.getUsername() == dest) {
            ip = u.getIp();
            port = u.getPort();
            valid_dest = true;
            break;
        }
    }
    if(!valid_dest) {
        sendMessageRaw("Error: Invalid destination");
        return false;
    }

    // Send the message if the user is logged in
    if(logged_in.count(dest) > 0 && !_send_message_to_usr(message, logged_in[dest])) {
        sendMessageRaw("Error: Message failed to send");
        return false;
    }
    // Store the message and messageID
    logfile << "[" << msg_id << "]" << sender << "->" << dest << ": " << message << "\n";
    logfile.flush();
    return true;
}

bool ServerPortal::_send_message_to_usr(std::string mesg, User usr) {
    // Set the remote_addr
    remote_addr = usr.getAddress();
    // Send the message itself
    return sendMessageRaw(mesg);
}

// TODO: Ensure no duplicate tokens in the program are created, it's not likely but it is possible
std::string ServerPortal::_generate_token() {
    const int TOKEN_LEN = 6;
    std::string token = "";
    for(int i = 0; i < TOKEN_LEN; i++)
        token += _generate_char();
    return token;
}

bool ServerPortal::_valid_token(std::string tok) {
    for(const User& u : users) {
        if(u.getToken().find(tok) != std::string::npos) // if the token is found in any user, return true
            return true;
    }
    // if we never found the toke, it must be invalid
    return false;
}

char ServerPortal::_generate_char() {
    //srand((unsigned int)time(nullptr));
    auto random_char = (unsigned char)(rand() % (126 - 33 + 1) + 33);
    return static_cast<char>(random_char);
}
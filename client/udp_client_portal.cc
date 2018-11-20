#include "udp_client_portal.h"

ClientPortal::ClientPortal() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
    }
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(32000);

    memset(&host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    host_addr.sin_port = htons(32000);
}

bool ClientPortal::loginToServer(std::string user, std::string pass, std::string ip, std::string port) {
    std::string login_request = user + "->server#login" + "<" + pass + ">" + "<" + ip + ">" + "<" + port + ">";
    sendMessageRaw(login_request);
    token = receiveMessageRaw(); // Server responds to login request with unique token for client
    if(token.find("Error") != std::string::npos) {
        // Print the error message
        std::cout << token << std::endl;
        return false;
    } else {
        this->user = user;
        this->pass = pass;
        return true;
    }
}

void ClientPortal::sendMessage(std::string content, std::string dest) {
    std::string message = this->user + "->" + dest + "#" + "<" + this->token + ">" + "<" + _generate_msg_id() + ">" + content;
    sendMessageRaw(message);
}

std::string ClientPortal::_generate_msg_id() {
    const int ID_LEN = 10;
    std::string id = "";
    for(int i = 0; i < ID_LEN; i++)
        id += _generate_char();
    return id;
}

char ClientPortal::_generate_char() {
    //srand((unsigned int)time(nullptr));
    unsigned char random_char = rand() % (126 - 33 + 1) + 33;
    return static_cast<char>(random_char);
}

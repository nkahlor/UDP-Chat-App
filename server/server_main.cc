#include <iostream>

#include "udp_server_portal.h"

ServerPortal serve = ServerPortal();
// Exit gracefully
void sighandler(int sig_num)
{
    serve.close();
    exit(0);
}

int main() {

    signal(SIGTSTP, sighandler);
    while(1) {
        // Receive a message
        std::string recv = serve.receiveMessageRaw();
        // Decide what to do based on the received message
        serve.parseMessage(recv);
    }

    return 0;
}
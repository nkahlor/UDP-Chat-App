#include <iostream>
#include <pthread.h>

#include "gui.h"

pthread_t receiver;
ClientPortal *client = ClientPortal::getInstance();
void *receive_messages(void*);

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    Layout window(nullptr);

    window.resize(800, 400);
    window.setWindowTitle("Chat Client");
    window.show();

    // Continually accept messages from the server
    pthread_create(&receiver, nullptr, receive_messages, &window);

    return app.exec();
}

void *receive_messages(void* data) {
    Layout* window = (Layout*) data;
    while(true) {
        std::string mesg = client->receiveMessageRaw();
        window->tabs->addMessage(mesg, window->tabs->currentIndex());
    }
}
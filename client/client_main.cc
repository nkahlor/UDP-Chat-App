#include <iostream>
#include <pthread.h>

#include "gui.h"

pthread_t receiver;
ClientPortal client;
void *receive_messages(void*);

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    Layout window(nullptr, &client);
    LoginDialog login(nullptr, &client);

    window.resize(800, 400);
    window.setWindowTitle("Chat Client");
    window.show();

    login.resize(400, 150);
    login.setWindowTitle("Login To Server");
    login.setModal(true);
    login.show();

    // Continually accept messages from the server
    pthread_create(&receiver, nullptr, receive_messages, &window);

    return app.exec();
}

void *receive_messages(void* data) {
    Layout* window = (Layout*) data;
    while(true) {
        QString new_message = QString::fromStdString(client.receiveMessageRaw());
        QListWidget* list = window->chat_tabs[window->tabs->currentIndex()]->messages;
        list->addItem(new_message);
    }
}
//
// Created by Nicholas Kahlor on 11/20/18.
//

#ifndef CLIENT_SERVER_CHAT_GUI_H
#define CLIENT_SERVER_CHAT_GUI_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QFormLayout>

#include <iostream>
#include "udp_client_portal.h"

class ChatTab : public QWidget {
public:
    ClientPortal client;
    QListWidget *messages;
    ChatTab(QWidget *parent = nullptr, ClientPortal *client = nullptr);

};

class Layout : public QWidget {
    Q_OBJECT;
public:
    ClientPortal client;
    QLineEdit *new_message;
    QTabWidget *tabs;

    QVector<ChatTab*> chat_tabs;

    Layout(QWidget* parent = nullptr, ClientPortal* client = nullptr);
    virtual ~Layout() = default;

public slots:
    void send_message_slot();
    void logout_slot();
};

class LoginDialog : public QDialog {
    Q_OBJECT
    ClientPortal client;
public:
    QLineEdit *pass_entry;
    QLineEdit *user_entry;
    QLabel *err;

    LoginDialog(QWidget *parent = nullptr, ClientPortal* client = nullptr);
    virtual ~LoginDialog() = default;

public slots:
    void login_btn_slot();
};

#endif //CLIENT_SERVER_CHAT_GUI_H

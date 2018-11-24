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
Q_OBJECT;
public:
    QString tab_name;
    QListWidget *messages;
    ChatTab(QWidget *parent, QString tab_name);
};

class MessageTabs : public QTabWidget {
Q_OBJECT;
public:
    MessageTabs(QWidget* parent = nullptr);
    void addMessage(std::string message, int tab_indx);
    void addTab(std::string tab_name);

private:
    QVector<ChatTab*> chat_tabs;
};

class LoginDialog : public QDialog {
Q_OBJECT
public:
    QLineEdit *pass_entry;
    QLineEdit *user_entry;
    QLabel *err;
    ClientPortal *client;

    LoginDialog(QWidget*);
    virtual ~LoginDialog() = default;

public slots:
    void login_btn_slot();
};

class Layout : public QWidget {
Q_OBJECT;
public:
    QLineEdit *new_message;
    MessageTabs *tabs;
    LoginDialog *login;
    ClientPortal *client;

    Layout(QWidget*);
    virtual ~Layout() = default;

public slots:
    void send_message_slot();
    void logout_slot();
};

#endif //CLIENT_SERVER_CHAT_GUI_H

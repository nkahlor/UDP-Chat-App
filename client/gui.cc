//
// Created by Nicholas Kahlor on 11/20/18.
//

#include "gui.h"

/* =========================================================
 *  Chat Tab
 * ========================================================= */
ChatTab::ChatTab(QWidget *parent, ClientPortal *client) {
    this->client = *client;

    QVBoxLayout *center = new QVBoxLayout();
    messages = new QListWidget();

    center->addWidget(messages);

    setLayout(center);
}

/* =========================================================
 *  Layout
 * ========================================================= */
Layout::Layout(QWidget *parent, ClientPortal* client_ptr)  : QWidget(parent) {
    this->client = *client_ptr;
    QGridLayout *main = new QGridLayout();
    QHBoxLayout *bottom = new QHBoxLayout();

    tabs = new QTabWidget();

    QPushButton *logout_btn = new QPushButton("Logout");
    QPushButton *send_btn = new QPushButton("Send");

    new_message = new QLineEdit(this);
    ChatTab* serverTab = new ChatTab(nullptr, client_ptr);
    ChatTab* allTab = new ChatTab(nullptr, client_ptr);
    this->chat_tabs.push_back(serverTab);
    this->chat_tabs.push_back(allTab);

    tabs->addTab(serverTab, "Server");
    tabs->addTab(allTab, "All");

    new_message->setStyleSheet("QLineEdit { color: black; }");

    bottom->addWidget(new_message);
    bottom->addWidget(send_btn);
    bottom->addWidget(logout_btn);

    main->addWidget(tabs, 0, 0);
    main->addLayout(bottom, 1, 0);

    QObject::connect(send_btn, &QPushButton::clicked,
                     this, &Layout::send_message_slot);

    QObject::connect(logout_btn, &QPushButton::clicked,
                     this, &Layout::logout_slot);

    setLayout(main);
}

void Layout::send_message_slot() {
    std::cout << "Attempting transmission to " << tabs->tabText(tabs->currentIndex()).toUtf8().constData() << std::endl;

}

void Layout::logout_slot() {
    std::cout << "Logging user out\n";
    client.logoutOfServer();
}

/* =========================================================
 *  Login Dialog
 * ========================================================= */
LoginDialog::LoginDialog(QWidget *parent, ClientPortal* client_ptr) : QDialog(parent) {
        this->client = *client_ptr;
        QFormLayout *login_form = new QFormLayout();
        QLabel *user = new QLabel("Username");
        QLabel *pass = new QLabel("Password");
        err = new QLabel("");
        user_entry = new QLineEdit(this);
        pass_entry = new QLineEdit(this);
        QPushButton *login = new QPushButton("Login");

        user_entry->setEchoMode(QLineEdit::EchoMode::Normal);
        pass_entry->setEchoMode(QLineEdit::EchoMode::Password);

        user_entry->setStyleSheet("QLineEdit { color: black; }");
        pass_entry->setStyleSheet("QLineEdit { color: black; }");

        login_form->addWidget(user);
        login_form->addWidget(user_entry);
        login_form->addWidget(pass);
        login_form->addWidget(pass_entry);
        login_form->addWidget(login);
        login_form->addWidget(err);

        QObject::connect(login, &QPushButton::clicked,
                         this, &LoginDialog::login_btn_slot);

        setLayout(login_form);
}

void LoginDialog::login_btn_slot() {
    if(client.loginToServer(user_entry->text().toUtf8().constData(), pass_entry->text().toUtf8().constData(), "120.0.0.1", "22"))
        this->close();
    else
        err->setText("Invalid username or password");
}
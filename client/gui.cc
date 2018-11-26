//
// Created by Nicholas Kahlor on 11/20/18.
//

#include "gui.h"

void Receiver::_parse_message(const std::string& mesg) {
    // If the message is an error/success echo display it in the 'Server' tab
    if(mesg.find("Error") != std::string::npos || mesg.find("Success") != std::string::npos) {
       // window->tabs->addMessage(mesg, 0);
    } else if(mesg.find("NewUser") != std::string::npos) {
        size_t new_user_begin = mesg.find_first_of(":") + 2;
        size_t new_user_end = mesg.size();
        std::string tab_name = mesg.substr(new_user_begin, new_user_end - new_user_begin);
        emit requestNewTab(QString::fromStdString(tab_name));
    } else {
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
            emit requestNewTab(QString::fromStdString(sender));
            emit requestNewMessage(QString::fromStdString(message), QString::fromStdString(sender));
        } catch (const std::out_of_range &e) {}

    }
}

void Receiver::run() {
    ClientPortal* client = ClientPortal::getInstance();
    while(true) {
        std::string mesg =  client->receiveMessageRaw();
        _parse_message(mesg);
    }
}


/* =========================================================
 *  Chat Tab
 * ========================================================= */
ChatTab::ChatTab(QWidget *parent, QString tab_name) : QWidget(parent) {
    QVBoxLayout *center = new QVBoxLayout();
    messages = new QListWidget();
    messages->setStyleSheet("QListWidget { color: black; }");

    center->addWidget(messages);

    this->tab_name = tab_name;

    setLayout(center);
}

/* =========================================================
 *  Message Tabs
 * ========================================================= */
MessageTabs::MessageTabs(QWidget *parent) : QTabWidget(parent) { }

void MessageTabs::addTab(std::string tab_name) {
    // Only add the tab if it isn't already there (no duplicates)
    if(findTab(tab_name) == -1) {
        ChatTab *newTab = new ChatTab(nullptr, QString::fromStdString(tab_name));
        this->chat_tabs.push_back(newTab);
        this->clear();
        for (ChatTab *tab : chat_tabs)
            QTabWidget::addTab(tab, tab->tab_name);
    }
}

void MessageTabs::addMessage(std::string message, int tab_indx) {
    // Update the listwidget associated with the tab_index
    QListWidget *curr_list = this->chat_tabs[tab_indx]->messages;
    curr_list->addItem(QString::fromStdString(message));
    std::string sender = message.substr(0, message.find_first_of(':'));
    // Make the sender's color red, and the receiver blue
    if(this->chat_tabs[tab_indx]->tab_name.toStdString() == sender)
        curr_list->item(curr_list->count() - 1)->setForeground(Qt::red);
    else
        curr_list->item(curr_list->count() - 1)->setForeground(Qt::blue);
}

int MessageTabs::findTab(std::string tab_name) {
    for(auto i = 0; i < chat_tabs.size(); i++)
        if(chat_tabs[i]->tab_name.toStdString() == tab_name)
            return i;
     return -1;
}

/* =========================================================
 *  Layout
 * ========================================================= */
Layout::Layout(QWidget *parent)  : QWidget(parent) {
    login = new LoginDialog(nullptr);
    this->client = ClientPortal::getInstance();
    QGridLayout *main = new QGridLayout();
    QHBoxLayout *bottom = new QHBoxLayout();

    QPushButton *logout_btn = new QPushButton("Logout");
    QPushButton *send_btn = new QPushButton("Send");

    new_message = new QLineEdit(this);

    tabs = new MessageTabs();

    tabs->addTab("Server");
    tabs->addTab("All");

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

    login->resize(400, 150);
    login->setWindowTitle("Login To Server");
    login->setModal(true);
    login->show();

    // Set up worker thread
    thread = new Receiver(this);
    QObject::connect(thread, &Receiver::requestNewTab, this, &Layout::createNewTab);
    QObject::connect(thread, &Receiver::requestNewMessage, this, &Layout::addNewMessage);
    QObject::connect(thread, &Receiver::finished, thread, &QObject::deleteLater);

    thread->start();

    setLayout(main);
}

void Layout::send_message_slot() {
    std::string dest = tabs->tabText(tabs->currentIndex()).toStdString();
    std::string msg_to_send = new_message->text().toStdString();
    std::string msg_to_display = client->getUser() + ": " + msg_to_send;
    tabs->addMessage(msg_to_display, tabs->currentIndex());
    client->sendMessage(msg_to_send, dest);
}

void Layout::logout_slot() {
    std::cout << "Logging user out\n";
    client->logoutOfServer();
    thread->terminate();
    QWidget::close();
}

// Every time a new user joins the chat, this will be called
void Layout::createNewTab(const QString& name) {
    tabs->addTab(name.toStdString());
}

void Layout::addNewMessage(const QString &message, const QString& sender) {
    int received_from_tab = tabs->findTab(sender.toStdString());
    if(received_from_tab != -1) {
        tabs->addMessage(message.toStdString(), received_from_tab);
     }
}

/* =========================================================
 *  Login Dialog
 * ========================================================= */
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
        this->client = ClientPortal::getInstance();
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
    if(client->loginToServer(user_entry->text().toUtf8().constData(), pass_entry->text().toUtf8().constData(), "120.0.0.1", "22"))
        QWidget::close();
    else
        err->setText("Invalid username or password");
}
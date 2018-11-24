//
// Created by Nicholas Kahlor on 11/20/18.
//

#include "gui.h"

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
MessageTabs::MessageTabs(QWidget *parent) : QTabWidget(parent) {

}

void MessageTabs::addTab(std::string tab_name) {
    ChatTab *newTab = new ChatTab(nullptr, QString::fromStdString(tab_name));
    this->chat_tabs.push_back(newTab);
    this->clear();
    for(ChatTab *tab : chat_tabs)
        QTabWidget::addTab(tab, tab->tab_name);
}

void MessageTabs::addMessage(std::string message, int tab_indx) {
    // Update the listwidget associated with the tab_index
    this->chat_tabs[tab_indx]->messages->addItem(QString::fromStdString(message));
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

    setLayout(main);
}

void Layout::send_message_slot() {
    std::string dest = tabs->tabText(tabs->currentIndex()).toUtf8().constData();
    std::cout << "Attempting transmission to " <<  dest << std::endl;
    client->sendMessage(new_message->text().toUtf8().constData(), dest);
}

void Layout::logout_slot() {
    std::cout << "Logging user out\n";
    client->logoutOfServer();
    QWidget::close();
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
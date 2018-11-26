#include <iostream>
#include <QThread>

#include "gui.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    Layout window(nullptr);

    window.resize(800, 400);
    window.setWindowTitle("Chat Client");
    window.show();

    return app.exec();
}

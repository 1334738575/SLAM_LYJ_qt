#include "qt_lyj.h"
#include <QApplication>
#include <QPushButton>
#include "Windows/WindowSLyj.h"

int testQT()
{
    int argc = 0;
    char **argv = nullptr;
    QApplication app(argc, argv);

    QPushButton button("Hello, Qt!");
    button.resize(200, 100);
    button.show();

    WindowsLyj window;
    window.show();

    return app.exec();
}
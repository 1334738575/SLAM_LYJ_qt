#include "WindowsLyj.h"
#include <QLabel>

WindowsLyj::WindowsLyj(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("My Qt Application");
    setFixedSize(400, 300);

    QLabel *label = new QLabel("Hello, Qt!", this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
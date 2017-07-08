#include "dialog.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QtSerialPort/QSerialPortInfo>

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);

    Dialog dialog;
    dialog.show();

    return a.exec();
}


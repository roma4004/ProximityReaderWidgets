#include "dialog.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QtSerialPort/QSerialPortInfo>
#include "dbconn.h"
#include "serialport.h"

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);
    if (!createConnSQLite() ) return 1;

    SerialPort ComPort;
    Dialog dialog;

    QThread *serialThread = new QThread;
    ComPort.moveToThread(serialThread);
    serialThread->start();

    dialog.show();

    return a.exec();
}


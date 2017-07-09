#include "dialog.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QtSerialPort/QSerialPortInfo>
#include "dbconn.h"

int main(int argc, char *argv[])
{       
    QApplication a(argc, argv);
    if (!createConnSQLite() ) return 1;

    Dialog dialog;
    dialog.show();

    return a.exec();
}


#include "serialport.h"
#include <QtSerialPort/QSerialPort>
#include <QTime>

SerialPort::SerialPort()
{

}

void SerialPort::run()
{

}

void SerialPort::transaction(const QString &portName, int waitTimeout,
                             const QString &request)
{
    QSerialPort ComPort;
    if (currentPortName != portName) {
        ComPort.close();
        currentPortName = portName;
    }
    currentWaitTimeout = waitTimeout;
    currentRequest = request;

    ComPort.setPortName(currentPortName);
    ComPort.setBaudRate(QSerialPort::Baud19200);

    if (!ComPort.open(QIODevice::ReadWrite)) {
        //emit error(tr("Не удалось открыть %1, код ошибки %2")
        //           .arg(portName).arg(ComPort.error() ) );
        return;
    }

    QByteArray currentRequestBA = currentRequest.toLocal8Bit();
    QByteArray currentRequestHex = currentRequestBA.toHex();
   // QByteArray requestData = currentRequest.toLocal8Bit();


    ComPort.write(currentRequestHex);                          // write request
    if (ComPort.waitForBytesWritten(waitTimeout)) {          // wait for write
        if (ComPort.waitForReadyRead(currentWaitTimeout)) {  // read for read
            QByteArray responseData = ComPort.readAll();     // read response
            while (ComPort.waitForReadyRead(10))
                responseData += ComPort.readAll();

            QByteArray responseHex = QByteArray::fromHex(responseData);
            QString response(responseHex.toHex(' ') );

            this->lastResponse = response;   //save last response
            //emit setResponseTxt(response);   //show response in label

        }// else emit timeout(1); //code 1 = recieve timeout
    }// else     emit timeout(2); //code 2 = send    timeout

    ComPort.close();
}

#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <QThread>
#include <QtSerialPort/QSerialPort>

class SerialPort : public QThread
{
    Q_OBJECT
public:
    explicit SerialPort();
    QString lastResponse;
    void run();

public slots:
    void transaction(const QString &portName, int waitTimeout,
                     const QString &request);

private:
    QString currentPortName;   
    QString currentRequest;
    int     currentWaitTimeout;

};

#endif // SERIALPORT_H

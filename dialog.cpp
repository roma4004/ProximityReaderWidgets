/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>

#include <QtSerialPort/QSerialPortInfo>

QT_USE_NAMESPACE

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , transactionCount(0)
    , serialPortLabel    (new QLabel     (tr("Serial port:"        )))
    , serialPortComboBox (new QComboBox  (                          ))
    , waitResponseLabel  (new QLabel     (tr("Wait response, msec:")))
    , waitResponseSpinBox(new QSpinBox   (                          ))
    , requestLabel       (new QLabel     (tr("Request:"            )))
    , requestLineEdit    (new QLineEdit  (tr("Who are you?"        )))
    , trafficLabel       (new QLabel     (tr("No traffic."         )))
    , statusLabel        (new QLabel     (tr("Status: Not running.")))
    , runButton          (new QPushButton(tr("Start"               )))
    , pingButton         (new QPushButton(tr("Detect device (Ping)"           )))
    , portInfoLabel      (new QLabel     (tr("PortInfo:"           )))
{
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        serialPortComboBox->addItem(info.portName() );
    displayPortInfo(serialPortComboBox->currentText() );
    waitResponseSpinBox->setRange(0, 10000);
    waitResponseSpinBox->setValue(1000);

    auto mainLayout = new QGridLayout;
    mainLayout->addWidget(serialPortLabel,     0, 0      );
    mainLayout->addWidget(serialPortComboBox,  0, 1      );
    mainLayout->addWidget(waitResponseLabel,   1, 0      );
    mainLayout->addWidget(waitResponseSpinBox, 1, 1      );
    mainLayout->addWidget(runButton,           0, 2      );
    mainLayout->addWidget(pingButton,          1, 2      );
    mainLayout->addWidget(requestLabel,        2, 0      );
    mainLayout->addWidget(requestLineEdit,     2, 1, 1, 3);
    mainLayout->addWidget(trafficLabel,        3, 0, 1, 4);
    mainLayout->addWidget(statusLabel,         4, 0, 1, 5);
    mainLayout->addWidget(portInfoLabel,       5, 0, 1, 5);
    setLayout(mainLayout);

    setWindowTitle(tr("Blocking Master"));
    serialPortComboBox->setFocus();

    connect(         runButton,  &QPushButton::clicked           , this   , &Dialog::transaction    );
    connect(        pingButton,  &QPushButton::clicked           , this   , &Dialog::pingCommand    );
    connect(           &thread, &MasterThread::response          , this   , &Dialog::showResponse   );
    connect(           &thread, &MasterThread::error             , this   , &Dialog::processError   );
    connect(           &thread, &MasterThread::timeout           , this   , &Dialog::processTimeout );
    connect(           &thread, &MasterThread::portChanged       , this   , &Dialog::displayPortInfo);
    connect(serialPortComboBox,    &QComboBox::currentTextChanged, this   , &Dialog::displayPortInfo);
}

void Dialog::transaction()
{
    setControlsEnabled(false);  //disable controls while doing transaction
    statusLabel->setText(tr("Status: Running, connected to port %1.")
                         .arg(serialPortComboBox->currentText() ) );
    thread.transaction(serialPortComboBox->currentText(),
                       waitResponseSpinBox->value(),
                       requestLineEdit->text());
}

void Dialog::showResponse(const QString &s)
{
    setControlsEnabled(true);
    trafficLabel->setText(tr("Traffic, transaction #%1:"
                             "\n\r-request: %2"
                             "\n\r-response: %3")
                          .arg(++transactionCount)
                          .arg(requestLineEdit->text() )
                          .arg(s));
}

void Dialog::processError(const QString &s)
{
    setControlsEnabled(true);
    statusLabel->setText(tr("Status: Not running, %1.").arg(s) );
    trafficLabel->setText(tr("No traffic.") );
}

void Dialog::processTimeout(const QString &s)
{
    setControlsEnabled(true);
    statusLabel->setText(tr("Status: Running, %1.").arg(s) );
    trafficLabel->setText(tr("No traffic.") );
}

void Dialog::setControlsEnabled(bool enable)
{
              runButton->setEnabled(enable);
             pingButton->setEnabled(enable);
     serialPortComboBox->setEnabled(enable);
    waitResponseSpinBox->setEnabled(enable);
        requestLineEdit->setEnabled(enable);
}
void Dialog::displayPortInfo(QString portName)
{
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        if (info.portName() == portName){
            QString s = QObject::tr("Port info: ")    + "\n"
                      + QObject::tr("Port: ")         + info.portName() + "\n"
                      + QObject::tr("Location: ")     + info.systemLocation() + "\n"
                      + QObject::tr("Description: ")  + info.description() + "\n"
                      + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                      + QObject::tr("Serial num: ")   + info.serialNumber() + "\n"
                      + QObject::tr("Vendor ID: ")    + (info.hasVendorIdentifier()
                                                        ? QString::number(info.vendorIdentifier(), 16)
                                                        : QString()) + "\n"
                      + QObject::tr("Product ID: ")   + (info.hasProductIdentifier()
                                                        ? QString::number(info.productIdentifier(), 16)
                                                        : QString()) + "\n"
                      + QObject::tr("Busy: ")         + (info.isBusy()
                                                        ? QObject::tr("Yes")
                                                        : QObject::tr("No")) + "\n";
            portInfoLabel->setText(s);
            return;
        }
    }
    portInfoLabel->setText("Port info for " + portName + " not found");

}
void Dialog::pingCommand()
{  //work only if device address = 85 (0x55)
    setControlsEnabled(false);  //disable controls while doing transaction
    statusLabel->setText(tr("Status: Running, connected to port %1.")
                         .arg(serialPortComboBox->currentText() ) );
    const auto infos = QSerialPortInfo::availablePorts();
    unsigned short int portCnt = 0;

    for (const QSerialPortInfo &info : infos) {
        thread.transaction(info.portName(),
                           waitResponseSpinBox->value(),
                           "55 64 09 50 52 2d 30 31 20 55 53 42 c9 11");
        if (thread.lastResponse == "55 65 00 4a 80"){
            serialPortComboBox->setCurrentIndex(portCnt);   //set active com port if received ping response
            return;
        }
        portCnt++;
    }
}

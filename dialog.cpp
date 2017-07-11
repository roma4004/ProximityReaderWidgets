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
#include <QtSql>
#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets>
#include <QTime>

QT_USE_NAMESPACE

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , transactionCount(0)
    , serialPortLabel    (new QLabel     (tr("Порт обмена:"              )))
    , serialPortComboBox (new QComboBox  (                                ))
    , waitResponseLabel  (new QLabel     (tr("Время ожидания ответа, мс:")))
    , waitResponseSpinBox(new QSpinBox   (                                ))
    , requestLabel       (new QLabel     (tr("Запрос:"                   )))
    , requestLineEdit    (new QLineEdit  (tr("Введите запрос"            )))
    , trafficLabel       (new QLabel     (tr("Нет передачи"              )))
    , requestEdit        (new QLineEdit  (tr("Текст запрос: "            )))
    , responseEdit       (new QLineEdit  (tr("Текст ответа: "            )))
    , statusLabel        (new QLabel     (tr("Статус: не запущен."       )))
    , runButton          (new QPushButton(tr("Отправить"                 )))
    , pingButton         (new QPushButton(tr("  Найти считыватель  "     )))
    , readCardButton     (new QPushButton(tr("Считать карту"             )))
    , addCardButton      (new QPushButton(tr("  Добавить карту  "        )))
    , portInfoLabel      (new QLabel     (tr("О устройстве:"             )))
{
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        serialPortComboBox->addItem(info.portName() );
    displayPortInfo(serialPortComboBox->currentText() );
    waitResponseSpinBox->setRange(0, 10000);
    waitResponseSpinBox->setValue(3000);

    model = new QSqlTableModel(this);
    model->setTable("proximityCards");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    QTableView *view = new QTableView;
    view->setModel(model);
    view->resizeColumnsToContents();

    submitButton = new QPushButton(tr("Submit")  );
    revertButton = new QPushButton(tr("&Revert") );

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(submitButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ActionRole);

    auto mainLayout = new QGridLayout;
    mainLayout->addWidget(serialPortLabel,     0, 0      );
    mainLayout->addWidget(serialPortComboBox,  0, 1      );
    mainLayout->addWidget(pingButton,          0, 2      );
    mainLayout->addWidget(addCardButton,       0, 3      );

    mainLayout->addWidget(waitResponseLabel,   1, 0      );
    mainLayout->addWidget(waitResponseSpinBox, 1, 1      );
    mainLayout->addWidget(readCardButton,      1, 2      );
    mainLayout->addWidget(runButton,           1, 3      );

    mainLayout->addWidget(requestLabel,        2, 0      );
    mainLayout->addWidget(requestLineEdit,     2, 1, 1, 3);

    mainLayout->addWidget(trafficLabel,        3, 0, 1, 5);
    mainLayout->addWidget(requestEdit,         4, 0, 1, 5);
    mainLayout->addWidget(responseEdit,        5, 0, 1, 5);

    mainLayout->addWidget(statusLabel,         6, 0, 1, 5);

    mainLayout->addWidget(portInfoLabel,       7, 0, 1, 5);

    mainLayout->addWidget(view,                8, 0, 1, 4);
  //mainLayout->addWidget(buttonBox,           9, 4, 1, 1);
  //addCardButton->setEnabled(false);
    setLayout(mainLayout);

    setWindowTitle(tr("Proximity reader"));
    serialPortComboBox->setFocus();

    connect(         runButton,  &QPushButton::clicked           ,  this, &Dialog::sendCommand       );
    connect(        pingButton,  &QPushButton::clicked           ,  this, &Dialog::pingCommandAllPort);
    connect(    readCardButton,  &QPushButton::clicked           ,  this, &Dialog::readCardCommand   );
    connect(     addCardButton,  &QPushButton::clicked           ,  this, &Dialog::addCardCommand    );
    connect(      submitButton,  &QPushButton::clicked           ,  this, &Dialog::submit            );
    connect(      revertButton,  &QPushButton::clicked           , model, &QSqlTableModel::revertAll );
    connect(           &thread, &MasterThread::response          ,  this, &Dialog::showResponse      );
    connect(           &thread, &MasterThread::error             ,  this, &Dialog::processError      );
    connect(           &thread, &MasterThread::timeout           ,  this, &Dialog::processTimeout    );
    connect(           &thread, &MasterThread::portChanged       ,  this, &Dialog::displayPortInfo   );

    connect(           &thread, &MasterThread::setRequestTxt     ,   requestEdit, &QLineEdit::setText);
    connect(           &thread, &MasterThread::setResponseTxt    ,  responseEdit, &QLineEdit::setText);

    connect(serialPortComboBox,    &QComboBox::currentTextChanged,  this, &Dialog::displayPortInfo   );
}

void Dialog::sendCommand()
{
    setControlsEnabled(false);  //disable controls while doing transaction
    statusLabel->setText(tr("Статус: отправка запроса через %1.")
                         .arg(serialPortComboBox->currentText() ) );
    thread.transaction(serialPortComboBox->currentText(),
                       waitResponseSpinBox->value(),
                       requestLineEdit->text());
}

void Dialog::showResponse(const QString &s)
{
    setControlsEnabled(true);
    trafficLabel->setText(tr("Передача №: #%1:"
                             "\n\r-Отправлено: %2"
                             "\n\r-Получено: %3")
                          .arg(++transactionCount)
                          .arg(requestLineEdit->text() )
                          .arg(s) );
}

void Dialog::processError(const QString &s)
{
    setControlsEnabled(true);
    statusLabel->setText(tr("Статус: ошибка с кодом %1").arg(s) );
    trafficLabel->setText(tr("Нет передачи.") );
}

void Dialog::processTimeout(int msgCode)
{
    QString str;
    switch(msgCode){
        case 1: str = tr("Вышло время на ответа %1")
                      .arg(QTime::currentTime().toString());
        break;
        case 2: str = tr("Вышло время на отправки %1")
                      .arg(QTime::currentTime().toString());
        break;
    }
    setControlsEnabled(true);
    statusLabel->setText(tr("Статус: %1").arg(str) );
    trafficLabel->setText(tr("Нет передачи.") );
}

void Dialog::setControlsEnabled(bool enable)
{
              runButton->setEnabled(enable);
             pingButton->setEnabled(enable);
         readCardButton->setEnabled(enable);
     serialPortComboBox->setEnabled(enable);
    waitResponseSpinBox->setEnabled(enable);
        requestLineEdit->setEnabled(enable);
}
void Dialog::displayPortInfo(QString portName)
{
    //addCardButton->setEnabled(false);

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        if (info.portName() == portName){             //if found select port to show only his description
            QString s = QObject::tr("О устройстве: ")   + "\n"
                      + QObject::tr("Номер порта: ")    + info.portName() + "\n"
                      + QObject::tr("Расположение: ")   + info.systemLocation() + "\n"
                      + QObject::tr("Описание: ")       + info.description() + "\n"
                      + QObject::tr("Производитель: ")  + info.manufacturer() + "\n"
                      + QObject::tr("Серийный номер: ") + info.serialNumber() + "\n"
                      + QObject::tr("ID поставщика: ")  + (info.hasVendorIdentifier()
                                                        ? QString::number(info.vendorIdentifier(), 16)
                                                        : QString()) + "\n"
                      + QObject::tr("ID продукта: ")    + (info.hasProductIdentifier()
                                                        ? QString::number(info.productIdentifier(), 16)
                                                        : QString()) + "\n"
                      + QObject::tr("Занят: ")          + (info.isBusy()
                                                        ? QObject::tr("Да")
                                                        : QObject::tr("нет")) + "\n";
            portInfoLabel->setText(s);
            pingCommand(portName);
            return;
        }
    }
    portInfoLabel->setText(tr("Инфо об устройстве") + portName + tr(" не найдена") );

}
bool Dialog::pingCommand(QString portName)
{
     requestEdit->setText(tr(PING_REQ) );
    responseEdit->setText(tr("") );
    thread.transaction(portName,
                       waitResponseSpinBox->value(),                       
                       PING_REQ);
    //Maybe here need sleep() to get enough time to receive response
    //if (thread.lastResponse == "55 65 00 4a 80"){
    //        addCardButton->setEnabled(true);
    //    return true;
    //}
    return false;
}

void Dialog::pingCommandAllPort()
{
    setControlsEnabled(false);  //disable controls while doing transaction

    const auto infos = QSerialPortInfo::availablePorts(); //get available serial ports
    unsigned short int portCnt = 0;

    for (const QSerialPortInfo &info : infos) {        //ping each port to find device
        statusLabel->setText(tr("Статус: поиск считывателя на %1")
                             .arg(info.portName() ) );
        if(pingCommand(info.portName() ) ) {       //if find select this port in combobox
            serialPortComboBox->setCurrentIndex(portCnt);
            return;
        }
        portCnt++;
    }
}

void Dialog::readCardCommand()
{
    setControlsEnabled(false);  //disable controls while doing transaction
    statusLabel->setText(tr("Статус: попытка считывания карты на %1")
                         .arg(serialPortComboBox->currentText() ) );

    thread.transaction(serialPortComboBox->currentText(),
                       waitResponseSpinBox->value(),
                       GET_KEY_REQ);
}
void Dialog::addCardCommand()
{
    readCardCommand();
    int cardNum = 0;
    cardNum = thread.lastResponse.responseText.toInt();
    QSqlQuery query;
    //query.exec("INSERT INTO proximityCards VALUES("
    //           "NULL, "
    //           "5476574567, "
    //           "datetime('now'), "
    //           "'test name')");

    query.prepare("INSERT INTO proximityCards (card_number, creation_date, name) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(cardNum);
    query.addBindValue(QTime::currentTime().toString());
    query.addBindValue("Example Name");

    query.exec();
    model->revertAll();
    submit();
    //need send RESET_KEY_REQ to device
}

void Dialog::submit()
{
    model->database().transaction();
    if (model->submitAll() ) {
        model->database().commit();
    } else {
        model->database().rollback();
        QMessageBox::warning(this, tr("DB error"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text() ) );
    }
}

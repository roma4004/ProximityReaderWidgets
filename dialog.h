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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "masterthread.h"

QT_BEGIN_NAMESPACE

class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;

class QDialogButtonBox;
class QPushButton;
class QSqlTableModel;

QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);

    void showResponse  (const QString &s);
    void processError  (const QString &s);
    void processTimeout(int msgCode);


private slots:    

    void     sendCommand();
    bool     pingCommand(QString portName);
    void     pingCommandAllPort();
    void readCardCommand();
    void  addCardCommand();
    void submit();

private:
    void setControlsEnabled(bool enable);
    void displayPortInfo(QString portName);

private:    
    int transactionCount;

    QLabel      *serialPortLabel;
    QComboBox   *serialPortComboBox;
    QLabel      *waitResponseLabel;
    QSpinBox    *waitResponseSpinBox;
    QLabel      *requestLabel;
    QLineEdit   *requestLineEdit;
    QLabel      *trafficLabel;
    QLineEdit   *requestEdit;
    QLineEdit   *responseEdit;
    QLabel      *statusLabel;    
    QPushButton *runButton;
    QPushButton *pingButton;
    QPushButton *readCardButton;
    QPushButton *addCardButton;
    QLabel      *portInfoLabel;

    QPushButton *submitButton;
    QPushButton *revertButton;
    QDialogButtonBox *buttonBox;
    QSqlTableModel *model;

    MasterThread thread;

    const char          PING_REQ[14] = {(char)(0x55), (char)(0x64), (char)(0x09),   //service data
                                        (char)(0x50), (char)(0x52), (char)(0x2d),   //message part
                                        (char)(0x30), (char)(0x31), (char)(0x20),   //=^^^^^^^^^^=
                                        (char)(0x55), (char)(0x53), (char)(0x42),   //=^^^^^^^^^^=
                                        (char)(0xc9), (char)(0x11)               }; //CRC16
    const char           PING_RES[5] = {(char)(0x55), (char)(0x64), (char)(0x00),   //service data
                                        (char)(0x4b), (char)(0x10)               }; //CRC16
    const char      RESET_KEY_REQ[5] = {(char)(0x55), (char)(0x65), (char)(0x00),   //service data
                                        (char)(0x4a), (char)(0x80)               }; //CRC16
    const char      RESET_KEY_RES[5] = {(char)(0x55), (char)(0x65), (char)(0x00),   //service data
                                        (char)(0x4a), (char)(0x80)               }; //CRC16
    const char        GET_KEY_REQ[5] = {(char)(0x55), (char)(0x66), (char)(0x00),   //service data
                                        (char)(0x4a), (char)(0x70)               }; //CRC16
    const char GET_KEY_RES_NOTKEY[5] = {(char)(0x55), (char)(0x66), (char)(0x00),   //service data
                                        (char)(0x4a), (char)(0x70)               }; //CRC16
    const char GET_KEY_RES_YESKEY[5] = {(char)(0x55), (char)(0x66), (char)(0x08) }; //service data, use only in check if request .contein(GET_KEY_REQ_YESKEY)

};

#endif // DIALOG_H

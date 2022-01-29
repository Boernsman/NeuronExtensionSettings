/*
 * This file is part of the Neuron Extension Settings application.
 * Copyright (c) 2022 Berhard Trinnes.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "writesettings.h"

#include <QSerialPort>
#include <QDebug>

WriteSettings::WriteSettings(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent) : QObject(parent)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
    m_master->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::StopBits::OneStop);
    m_master->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::DataBits::Data8);
    m_master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrate);
    m_master->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);

    m_master->setTimeout(1000);
    m_master->setNumberOfRetries(3);

    connect (m_master, &QModbusRtuSerialMaster::errorOccurred, this,  [this] {
        qDebug() << "   - Modbus RTU error:" << m_master->errorString();
    });
    connect (m_master, &QModbusRtuSerialMaster::stateChanged, this,  [] (QModbusDevice::State state) {

        bool connected = (state == QModbusDevice::State::ConnectedState);
        qDebug() << "   - Modbus connected state changed:" << connected;
    });
}

void WriteSettings::write(uint address, WriteSettings::Baudrate baud, WriteSettings::Parity parity)
{
    if (address < 1 || address > 255) {
        qWarning() << "Write settings: Address not valid" << address;
        return;
    }
    uint16_t valueReg0 = 0x0000; //Register 1027
    uint16_t valueReg1 = 0x0000; //Register 1028

    valueReg0 = baud; //first 12 bits

    if (parity != Parity::ParityNone) {
        valueReg0 |= (1 << 13);
        if (parity == ParityOdd) {
            valueReg0 |= (1 << 14);
        }
    }
    //valueReg0 |= (1 << 15);
    valueReg1 = address;

    if (!m_master->connectDevice()) {
        qWarning() << "Connecing to modbus RTU master failed";
        return;
    }


    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1027, 2);
    request.setValue(0, valueReg0);
    request.setValue(1, valueReg1);

    if (QModbusReply *reply = m_master->sendWriteRequest(request, 15)) {
        if (!reply->isFinished()) {
            QObject::connect(reply, &QModbusReply::finished,[=] {
                if (reply->error() == QModbusDevice::NoError) {
                    qDebug() << "Writting setting successfull" << reply->result().value(0);
                    QModbusDataUnit request2 = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1003, 1);
                    request2.setValue(0, 0x0001);
                    if (QModbusReply *reply2 = m_master->sendWriteRequest(request, 15)) {
                        if (!reply2->isFinished()) {
                            QObject::connect(reply2, &QModbusReply::finished,[=] {
                                if (reply2->error() == QModbusDevice::NoError) {
                                    qDebug() << "Storing the values was successfull" << reply2->result().value(0);
                                    qDebug() << "Power cycle the extension now!";
                                    emit writeFinished();
                                } else {
                                    qDebug() << "Error" << reply->errorString();
                                }
                            });
                        }
                    }
                } else {
                    qDebug() << "Error" << reply->errorString();
                }
            });
        }
    } else {
        qWarning() << "Error: " << m_master->errorString();
        return;
    }
}


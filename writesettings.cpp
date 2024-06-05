/*
 * This file is part of the Neuron Extension Settings application.
 * Copyright (c) 2022 Bernhard Trinnes.
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

void WriteSettings::write(uint address, uint writeAddress, WriteSettings::Baudrate writeBaud, WriteSettings::Parity writeParity)
{
    qDebug() << "Write";
    if (writeAddress < 1 || writeAddress > 255) {
        qWarning() << "     - Write address is not valid" << writeAddress;
        return;
    }
    uint16_t valueReg0 = 0x0000; //Register 1027
    uint16_t valueReg1 = writeAddress; //Register 1028

    valueReg0 = writeBaud; //first 12 bits

    if (writeParity != Parity::ParityNone) {
        valueReg0 |= (1 << 13);
        if (writeParity == ParityOdd) {
            valueReg0 |= (1 << 14);
        }
    }
    if (!m_master->connectDevice()) {
        qWarning() << "     - Connecing to modbus RTU master failed";
        return;
    }



    QModbusDataUnit readRequest = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1004, 1);
    if (QModbusReply *readReply = m_master->sendReadRequest(readRequest, address)) {
        if (!readReply->isFinished()) {
            connect(readReply, &QModbusReply::finished, this, [readReply, valueReg0, valueReg1, address, this] {
                int registerAddress = 0;

                quint16 result = readReply->result().value(0);
                if (result == 1) {
                    qDebug() << "     - Model xS10";
                    registerAddress = 1031;
                } else if (result == 784) {
                    qDebug() << "     - Model xS30";
                    registerAddress = 1034;
                } else if (result == 528) {
                    qDebug() << "     - Model xS40";
                    registerAddress = 1023;
                } else if (result == 5) {
                    qDebug() << "     - Model xS50";
                    registerAddress = 1023;
                } else if (result == 272) {
                    qDebug() << "     - Model xS11";
                    registerAddress = 1027;
                } else if (result == 273) {
                    qDebug() << "     - Model xS51";
                    registerAddress = 1023;
                } else {
                    qDebug() << "     - Unkown model" << result;
                    return;
                }

                QModbusDataUnit writeRequest = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, registerAddress, 2);
                writeRequest.setValue(0, valueReg0);
                writeRequest.setValue(1, valueReg1);

                if (QModbusReply *writeReply = m_master->sendWriteRequest(writeRequest, address)) {
                    if (!writeReply->isFinished()) {
                        connect(writeReply, &QModbusReply::finished, this, [writeReply, address, this] {
                            if (writeReply->error() == QModbusDevice::NoError) {
                                qDebug() << "Writing settings successfull";
                                qDebug() << "Sending store command";
                                QModbusDataUnit request2 = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1003, 1);
                                request2.setValue(0, 0x0001);
                                if (QModbusReply *reply2 = m_master->sendWriteRequest(request2, address)) {
                                    if (!reply2->isFinished()) {
                                        connect(reply2, &QModbusReply::finished, reply2, &QModbusReply::deleteLater);
                                        connect(reply2, &QModbusReply::finished, this, [reply2, this] {
                                            if (reply2->error() == QModbusDevice::NoError) {
                                                qDebug() << "Storing the values was successfull";
                                                qDebug() << "Power cycle the extension now!";
                                                emit writeFinished();
                                            } else {
                                                qDebug() << "Error" << reply2->errorString();
                                            }
                                        });
                                    } else {
                                        reply2->deleteLater();
                                    }
                                } else {
                                    qWarning() << "Error: " << m_master->errorString();
                                } // Storage command reply
                            } else {
                                qDebug() << "Error" << writeReply->errorString();
                            }
                        });
                    } else {
                        writeReply->deleteLater();
                    }
                } else {
                    qWarning() << "Error: " << m_master->errorString();
                    return;
                }
            });
        } else {
            qWarning() << "Error: " << m_master->errorString();
            return;
        }
    }
}


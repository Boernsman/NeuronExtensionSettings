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

#include "discovery.h"

#include <QDebug>
#include <QTimer>

Discovery::Discovery(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent) : QObject(parent)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
    m_master->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::StopBits::OneStop);
    m_master->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::DataBits::Data8);
    m_master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrate);
    m_master->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_master->setTimeout(200);
    m_master->setNumberOfRetries(0);

    connect (m_master, &QModbusRtuSerialMaster::errorOccurred, this,  [this] {
        qDebug() << "   - Modbus RTU error:" << m_master->errorString();
    });
    connect (m_master, &QModbusRtuSerialMaster::stateChanged, this,  [] (QModbusDevice::State state) {

        bool connected = (state == QModbusDevice::State::ConnectedState);
        qDebug() << "   - Modbus connected state changed:" << connected;
    });
}

bool Discovery::startDiscovery(int startAddress, int endAddress)
{
    qDebug() << "Start Discovery";
    qDebug() << "   - Start Address" << startAddress;
    qDebug() << "   - End address" << endAddress;

    if (!m_master->connectDevice()) {
        qDebug() << "Could not connect to port";
        return false;
    }

    if (m_discoveryOngoing) {
        qDebug() << "Discovery is already in progress";
        return false;
    }

    if (startAddress > endAddress) {
        qDebug() << "Start address must be higher than end address";
        return false;
    }

    m_endAddress = endAddress;
    m_sweepingAddress = startAddress;

    getNext(m_sweepingAddress);
    m_discoveryOngoing = true;
    return true;
}

void Discovery::stopDiscovery()
{
    qDebug() << "Stopping discovery";
    m_sweepingAddress = 1;
    m_discoveryOngoing = false;
}

void Discovery::getNext(int address)
{
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1000, 7);
    qDebug() << "   - Probing address" << address;
    if (QModbusReply *reply = m_master->sendReadRequest(request, address)) {
        if (!reply->isFinished()) {
            //connect(reply, &QModbusReply::finished, reply, &QModbusReply::deleteLater);
            connect(reply, &QModbusReply::finished, this, [this, reply] {
                if (reply->serverAddress() == m_sweepingAddress) {
                    m_sweepingAddress = reply->serverAddress()+1;
                } else if (reply->serverAddress() < m_sweepingAddress){
                    // A reply returns multiple finish signals depending on the retry
                    qWarning() << "     - Got modbus reply from previous request, ignoring";
                    return;
                }

                QVector<quint16> result = reply->result().values();
                if (result.length() == 7) {
                    qDebug() << "Found Extension";
                    qDebug() << "     - Slave Address" << reply->serverAddress();
                    qDebug() << "     - Serial number" << (static_cast<quint32>(result[6])<<16 | result[5]);

                    if (result[4] == 1) {
                        qDebug() << "     - Model xS10";
                    } else if (result[4] == 784) {
                        qDebug() << "     - Model xS30";
                    } else if (result[4] == 528) {
                        qDebug() << "     - Model xS40";
                    } else if (result[4] == 5) {
                        qDebug() << "     - Model xS50";
                    } else if (result[4] == 272) {
                        qDebug() << "     - Model xS11";
                    } else if (result[4] == 273) {
                        qDebug() << "     - Model xS51";
                    } else {
                        qDebug() << "     - Unkown model" << result[4];
                    }
                }
                if (reply->serverAddress() >= m_endAddress) {
                    m_discoveryOngoing = false;
                    qWarning() << "Discovery: Discovery finished";
                    emit discoveryFinished();
                } else {
                    if (m_discoveryOngoing)
                        getNext(m_sweepingAddress);
                }
            });
        } else {
            reply->deleteLater();
            qDebug() << "   - Reply finished immediatelly";
        }
    } else {
        qWarning() << "     - Read error: " << m_master->errorString();
    }
}

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

#include "test.h"

#include <QElapsedTimer>
#include <QDebug>

Test::Test(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent) : QObject(parent)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
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

void Test::start(int address, int cycles)
{
    m_address = address;
    m_cycles = cycles;
    m_testOngoing = true;
    m_errorCount = 0;
    m_averageElapsedTime = 0;
    setNext(0x01);
}

void Test::setNext(uint16_t value)
{
    QElapsedTimer timer;
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 31, 1);
    request.setValue(0, value);
    if (QModbusReply *reply = m_master->sendReadRequest(request, m_address)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, reply, &QModbusReply::deleteLater);
            connect(reply, &QModbusReply::finished, this, [this, value, timer, reply] {

                qDebug() << "Reply received, elapsed time" << timer.elapsed() << "milli-seconds";
                if (m_averageElapsedTime == 0) {
                    m_averageElapsedTime = timer.elapsed();
                } else {
                    m_averageElapsedTime = (m_averageElapsedTime + timer.elapsed())/2;
                }
                if (reply->error() != QModbusDevice::NoError) {
                    m_errorCount++;
                }
                uint16_t newValue;
                m_cycles--;
                if (m_cycles == 0) {
                    m_testOngoing = false;
                    qDebug() << "Test finished";
                    qDebug() << "   - Average cycle time:" << m_averageElapsedTime;
                    qDebug() << "   - Error count:" << m_errorCount;
                    emit testFinished();
                    return;
                }
                if (value & 0x10) {
                    newValue = value << 1;
                } else {
                    newValue = (value << 1) | 0x01;
                }
                setNext(newValue);
            });
        } else {
            reply->deleteLater();
            qDebug() << "   - Reply finished immediatelly";
        }
    } else {
        qWarning() << "     - Read error: " << m_master->errorString();
    }
}

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

#ifndef TEST_H
#define TEST_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent = nullptr);
    void start(int address, int cycles);

private:
    QModbusRtuSerialMaster *m_master = nullptr;

    int m_cycles = 0;
    int m_address = 1;
    qint64 m_averageElapsedTime;
    int m_errorCount;
    bool m_testOngoing = false;

    void setNext(uint16_t value);

signals:
    void testFinished();
};

#endif // TEST_H

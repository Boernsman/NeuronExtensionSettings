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

#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

class Discovery : public QObject
{
    Q_OBJECT
public:

    explicit Discovery(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent = nullptr);

    bool startDiscovery(int startAddress = 1, int endAddress = 7);
    void stopDiscovery();

private:
    QModbusRtuSerialMaster *m_master = nullptr;

    int m_endAddress;
    int m_sweepingAddress = 1;
    bool m_discoveryOngoing = false;

    void getNext(int address);

signals:
    void discoveryFinished();

};

#endif // DISCOVERY_H

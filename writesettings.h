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

#ifndef WRITESETTINGS_H
#define WRITESETTINGS_H

#include <QObject>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>

class WriteSettings : public QObject
{
    Q_OBJECT
public:
    enum Baudrate {
        Baudrate_2400 = 11,
        Baudrate_4800 = 12,
        Baudrate_9600 = 13,
        Baudrate_19200 = 14,
        Baudrate_38400 = 15,
        Baudrate_57600 = 4097,
        Baudrate_115200 = 4098
    };
    Q_ENUM(Baudrate)

    enum Parity {
        ParityEven,
        ParityOdd,
        ParityNone
    };
    Q_ENUM(Parity)

    explicit WriteSettings(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent = nullptr);
    void write(uint address, uint writeAddress, Baudrate writeBaud, Parity writeParity);

private:
    QModbusRtuSerialMaster *m_master = nullptr;

signals:
    void writeFinished();
};

#endif // WRITESETTINGS_H

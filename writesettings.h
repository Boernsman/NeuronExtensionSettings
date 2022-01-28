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

#ifndef WRITESETTINGS_H
#define WRITESETTINGS_H

#include <QObject>
#include <QModbusRtuSerialMaster>

class WriteSettings : public QObject
{
    Q_OBJECT
public:
    enum Baudrate {
        Baurate_2400 = 11,
        Baurate_4800 = 12,
        Baurate_9600 = 13,
        Baurate_19200 = 14,
        Baurate_38400 = 15,
        Baurate_57600 = 4097,
        Baudrate_115200 = 4098
    };
    Q_ENUM(Baudrate)

    enum Parity {
        ParityEven,
        ParityOdd,
        ParityNone
    };
    Q_ENUM(Parity)

    explicit WriteSettings(const QString &serialPort, QObject *parent = nullptr);

    void write(uint address, Baudrate baud, Parity parity);
private:
    QModbusRtuSerialMaster *m_master = nullptr;
signals:

};

#endif // WRITESETTINGS_H

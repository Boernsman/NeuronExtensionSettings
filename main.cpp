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

#include <QCoreApplication>
#include <QtSerialBus>
#include <QDebug>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "discovery.h"
#include "writesettings.h"
#include "test.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("NeuronExtensionSettings");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Neuron extension tool, to set baudrate, parity and address.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption verboseOption("verbose", "Verbose output.");
    parser.addOption(verboseOption);

    QCommandLineOption discoveryOption("discover", "Discover Neuron extensions.");
    parser.addOption(discoveryOption);

    QCommandLineOption writeOption("write", "Write settings to neuron extension.");
    parser.addOption(writeOption);

    QCommandLineOption testOption("test", "Test neuron extension connection.");
    parser.addOption(testOption);

    QCommandLineOption serialPortOption(QStringList() << "s" << "serial port", "System path to serial port", "serial port");
    parser.addOption(serialPortOption);

    QCommandLineOption parityOption(QStringList() << "p" << "parity", "Set parity even|none, 'none' is default", "parity");
    parser.addOption(parityOption);

    QCommandLineOption baudOption(QStringList() << "b" << "baudrates", "Set baudrate, 115200 default", "baudrate");
    parser.addOption(baudOption);

    QCommandLineOption addressOption(QStringList() << "a" << "address", "Set address, [1, 254] default 15", "address");
    parser.addOption(addressOption);

    QCommandLineOption writebaudOption(QStringList() << "wb" << "writebaudrate", "Set write baudrate, 115200 default", "baudrate");
    parser.addOption(writebaudOption);

    QCommandLineOption writeaddressOption(QStringList() << "wa" << "writeaddress", "Set write address, 1 default", "address");
    parser.addOption(writeaddressOption);

    QCommandLineOption writeparityOption(QStringList() << "wp" << "writeparity", "Set write parity even|none, 'even' is default", "parity");
    parser.addOption(writeparityOption);

    QCommandLineOption endaddressOption(QStringList() << "endaddress", "Set discovery end-address. 15 is default", "addresss");
    parser.addOption(endaddressOption);

    // Process the actual command line arguments given by the user
    parser.process(app);
    QTextStream qtin(stdin);

    QString portName;
    if (parser.isSet(serialPortOption)) {
        portName = parser.value(serialPortOption);
    } else {
        while (1) {
            qDebug() << "Select the serial port [1, "+QString::number(QSerialPortInfo::availablePorts().count())+"]:";

            int i = 0;
            QList<QSerialPortInfo> infoList;
            foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
                i++;
                qDebug() << i << info.portName();
                infoList.append(info);
            }
            qDebug() << "Enter number:";
            int j; qtin >> j;
            if (j < 1 || j > i) {
                qWarning() << "Selected number not available." << QString::number(j);
                continue;
            }

            if (infoList.at(j-1).isBusy()) {
                qDebug() << "Selected serial port is busy, select another one.";
                continue;
            }
            qDebug() << "The selected port is" << infoList.at(j-1).systemLocation();
            portName = infoList.at(j-1).systemLocation();
            break;
        }
    }

    int baudrate = 19200;
    if (parser.isSet(baudOption)) {
        bool ok;
        baudrate = parser.value(baudOption).toInt(&ok);
        if (!ok) {
            qDebug() << "Baudrate is not valid" << parser.value(baudOption);
        }
    } else {
        qDebug() << "Default baudrate:" << baudrate;
    }

    QSerialPort::Parity parity = QSerialPort::Parity::NoParity;
    if (parser.isSet(parityOption)) {
        QString parityValue = parser.value(parityOption);
        if (parityValue.startsWith("none")) {
            parity = QSerialPort::Parity::NoParity;
        } else if (parityValue.startsWith("even")) {
            parity = QSerialPort::Parity::EvenParity;
        } else if (parityValue.startsWith("odd")) {
            parity = QSerialPort::Parity::OddParity;
        } else {
            qDebug() << "Parity" << parityValue << "is not supported, must be 'even','odd' or 'none'";
        }
    } else {
        qDebug() << "Default parity:" << parity;
    }

    int address = 15;
    if (parser.isSet(addressOption)) {
        bool ok;
        address = parser.value(addressOption).toInt(&ok);
        if (!ok) {
            qDebug() << "Address is not valid" << parser.value(addressOption);
            return -1;
        }
    }

    if (parser.isSet(verboseOption)) {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
    }

    if (parser.isSet(discoveryOption)) {

        int endaddress = 15;
        if (parser.isSet(endaddressOption)) {
            bool ok;
            endaddress = parser.value(endaddressOption).toInt(&ok);
            if (!ok) {
                qDebug() << "Discovery end address is not valid" << parser.value(endaddressOption);
                return -1;
            }
        }
        qDebug() << "Discovery:";
        qDebug() << "   - Port" << portName;
        qDebug() << "   - Baud rate" << baudrate;
        qDebug() << "   - Parity" << parity;
        qDebug() << "   - Start address" << 1;
        qDebug() << "   - End address" << endaddress;
        Discovery *discover = new Discovery(portName, baudrate, parity);
        if (!discover->startDiscovery(1, endaddress)) {
            return -1;
        }
        QObject::connect(discover, &Discovery::discoveryFinished, [&app, discover] {
            discover->deleteLater();
            app.quit();
        });
    } else if (parser.isSet(writeOption)){

        int writeSettingAddress = 15;
        if (parser.isSet(writeaddressOption)) {
            bool ok;
            writeSettingAddress = parser.value(writeaddressOption).toInt(&ok);
            if (!ok) {
                qDebug() << "Write address is not valid" << parser.value(writeaddressOption);
                return -1;
            }
        }

        WriteSettings::Baudrate writeSettingBaudrate = WriteSettings::Baudrate::Baudrate_19200;
        if (parser.isSet(writebaudOption)) {

            bool ok;
            int writeBaudrate = parser.value(writebaudOption).toInt(&ok);
            if (!ok) {
                qDebug() << "Write baudrate is not valid" << parser.value(writebaudOption);
                return -1;
            }

            if (writeBaudrate == 2400) {
                writeSettingBaudrate = WriteSettings::Baudrate_2400;
            } else if (writeBaudrate == 4800) {
                writeSettingBaudrate= WriteSettings::Baudrate_4800;
            } else if (writeBaudrate == 9600) {
                writeSettingBaudrate = WriteSettings::Baudrate_9600;
            } else if (writeBaudrate == 19200) {
                writeSettingBaudrate = WriteSettings::Baudrate_19200;
            } else if (writeBaudrate == 38400) {
                writeSettingBaudrate = WriteSettings::Baudrate_38400;
            } else if (writeBaudrate == 57600) {
                writeSettingBaudrate = WriteSettings::Baudrate_57600;
            } else if (writeBaudrate == 115200) {
                writeSettingBaudrate = WriteSettings::Baudrate_115200;
            } else {
                qDebug() << "Write baudrate is not valid";
                return -1;
            }
        }
        WriteSettings::Parity writeSettingParity = WriteSettings::Parity::ParityEven;
        if (parser.isSet(writeparityOption)) {
            QString writeParityValue = parser.value(writeparityOption);
            if (writeParityValue.startsWith("none")) {
                writeSettingParity = WriteSettings::Parity::ParityNone;
            } else if (writeParityValue.startsWith("even")) {
                writeSettingParity = WriteSettings::Parity::ParityEven;
            } else if (writeParityValue.startsWith("odd")) {
                writeSettingParity = WriteSettings::Parity::ParityOdd;
            } else {
                qDebug() << "Write parity" << writeParityValue << "is not supported, must be 'even','none' or 'odd'";
                return -1;
            }
        } else {
            qDebug() << "Default write parity:" << writeSettingParity;
        }
        qDebug() << "Write settings";
        qDebug() << "   - Address:" << address;
        qDebug() << "   - Baudrate:" << baudrate;
        qDebug() << "   - Parity:" << parity;
        qDebug() << "   - Write address:" << writeSettingAddress;
        qDebug() << "   - Write baudrate:" << writeSettingBaudrate;
        qDebug() << "   - Write parity:" << writeSettingParity;
        WriteSettings *settings = new WriteSettings(portName, baudrate, parity);
        settings->write(address, writeSettingAddress, writeSettingBaudrate, writeSettingParity);
        QObject::connect(settings, &WriteSettings::writeFinished, [&app, settings] {
            settings->deleteLater();
            app.quit();
        });
    } else if (parser.isSet(testOption)){
        int testCycles = 100;
        qDebug() << "Teset connection";
        qDebug() << "   - Address:" << address;
        qDebug() << "   - Baudrate:" << baudrate;
        qDebug() << "   - Parity:" << parity;
        qDebug() << "   - Test cycles:" << testCycles;
        Test *test = new Test(portName, baudrate, parity);
        test->start(address, testCycles);
        QObject::connect(test, &Test::testFinished, [&app, test] {
            test->deleteLater();
            app.quit();
        });
    } else {
        qDebug() << "No discovery and no write option is set. Doing nothing.";
    }

    return app.exec();
}

#include <QCoreApplication>
#include <QtSerialBus>
#include <QDebug>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "discovery.h"
#include "writesettings.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("NeuronExtensionSettings");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Neuron extension tool, to set baudrate and address.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption verboseOption("verbose", "Verbose output.");
    parser.addOption(verboseOption);

    QCommandLineOption discoveryOption("discover", "Discover Neuron extensions.");
    parser.addOption(discoveryOption);

    QCommandLineOption writeOption("write", "Write settings to neuron extension.");
    parser.addOption(writeOption);

    QCommandLineOption serialPortOption(QStringList() << "s" << "serial port", "System path to serial port", "serial port");
    parser.addOption(serialPortOption);

    QCommandLineOption parityOption(QStringList() << "p" << "parity", "Set parity even|none, 'none' is default", "parity");
    parser.addOption(parityOption);

    QCommandLineOption baudOption(QStringList() << "b" << "baudrates", "Set baudrate, 115200 default", "baudrate");
    parser.addOption(baudOption);

    // Process the actual command line arguments given by the user
    parser.process(a);
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

    int baudrate = 115200;
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
        } else {
            qDebug() << "Parity" << parityValue << "is not supported, must be 'even' or 'none'";
        }
    } else {
        qDebug() << "Default parity:" << parity;
    }

    if (parser.isSet(verboseOption)) {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));
    }

    if (parser.isSet(discoveryOption)) {
        qDebug() << "Discovery: Port" << portName << "baud rate" << baudrate << "parity" << parity;
        Discovery *discover = new Discovery(portName, baudrate, parity);
        if (!discover->startDiscovery(1, 15)) {
            return -1;
        }
        QObject::connect(discover, &Discovery::discoveryFinished, &a, &QCoreApplication::quit);
    } else if (parser.isSet(writeOption)){
        WriteSettings *settings = new WriteSettings(portName);
        settings->write(3, WriteSettings::Baudrate_115200, WriteSettings::ParityEven);
    } else {
        qDebug() << "No discovery and no write option is set. Doing nothing.";
    }

    return a.exec();
}

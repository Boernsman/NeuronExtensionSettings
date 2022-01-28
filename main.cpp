#include <QCoreApplication>
#include <QtSerialBus>
#include <QDebug>
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

    // A boolean option with multiple names (-d, --discover)
    QCommandLineOption discoveryOption(QStringList() << "d" << "discover", "main", "Discover method");
    parser.addOption(discoveryOption);

    QCommandLineOption portOption(QStringList() << "s" << "serial port", "main", "Set port");
    parser.addOption(discoveryOption);

    QCommandLineOption parityOption(QStringList() << "p" << "parity", "main", "Set parity");
    parser.addOption(discoveryOption);

    QCommandLineOption baudOption(QStringList() << "b" << "baudrates", "main", "Set baudrate");
    parser.addOption(discoveryOption);

    QCommandLineOption writeOption(QStringList() << "w" << "write", "main", "Write settings to neuron extension.");
    parser.addOption(writeOption);

    // Process the actual command line arguments given by the user
    parser.process(a);
    QTextStream qtin(stdin);

    QString portName;
    if (!parser.isSet(portOption)) {
        qDebug() << "Select the serial port [1, "+QString::number(QSerialPortInfo::availablePorts().count())+"]:";
        int i = 0;
        QList<QSerialPortInfo> infoList;
        foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
            i++;
            qDebug() << QString::number(i) << info.portName();
            infoList.insert(i, info);
        }
        int j; qtin >> j;
        if (j < 1 || j > i) {
            qWarning() << "Selected port not available.";
            return -1;
        }

        if (infoList.at(j).isBusy()) {
            qDebug() << "Selected serial port is busy.";
            return -1;
        }
        portName = infoList.at(j).portName();
    } else {
        portName = parser.value(portOption);
    }

    bool ok;
    int baudrate = parser.value(baudOption).toInt(&ok);
    if (!ok) {
        qDebug() << "Baudrate is not valid" << parser.value(baudOption);
    }

    QSerialPort::Parity parity = QSerialPort::Parity::EvenParity;
    if (parser.isSet(parityOption)) {
        QString parityValue = parser.value(parityOption);
        if (parityValue.startsWith("none")) {
            parity = QSerialPort::Parity::NoParity;
        } else if (parityValue.startsWith("odd")) {
            parity = QSerialPort::Parity::OddParity;
        }
    }

    if (parser.isSet(discoveryOption)) {
        qDebug() << "Discovery: Port" << portName << "baud rate" << baudrate << "parity" << parity;
        Discovery discover(portName, baudrate, parity);
        discover.startDiscovery(1, 15);
    } else if (parser.isSet(writeOption)){
        WriteSettings settings(portName);
        settings.write(3, WriteSettings::Baudrate_115200, WriteSettings::ParityEven);
    }

    return a.exec();
}

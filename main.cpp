#include <QCoreApplication>
#include <QtSerialBus>
#include <QDebug>
#include <QSerialPort>

#include "discovery.h"
#include "writesettings.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("NeuronExtensionSettings");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("port", QCoreApplication::translate("main", "Serial port."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    // A boolean option with a single name (-p)
    QCommandLineOption showProgressOption("p", QCoreApplication::translate("main", "Show progress during copy"));
    parser.addOption(showProgressOption);

    // A boolean option with multiple names (-d, --discover)
    QCommandLineOption discoveryOption(QStringList() << "d" << "discover",
            QCoreApplication::translate("main", "Discover method"));
    parser.addOption(discoveryOption);

    QCommandLineOption writeOption(QStringList() << "w" << "write",
            QCoreApplication::translate("main", "Write settings to neuron extension."));
    parser.addOption(writeOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    if (parser.isSet(discoveryOption)) {
        Discovery discover("/dev/ttyNS0", 19200, QSerialPort::Parity::EvenParity);
        discover.startDiscovery(1, 15);
    } else if (parser.isSet(writeOption)){
        WriteSettings settings("/dev/ttyNS0");
        settings.write(3, WriteSettings::Baudrate_115200, WriteSettings::ParityEven);
    }




    return a.exec();
}

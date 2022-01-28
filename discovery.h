#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>

class Discovery : public QObject
{
    Q_OBJECT
public:

    enum ExtensionTypes {
        xS10,
        xS30,
        xS40,
        xS50,
        xS11,
        xS51,
        Unknown
    };
    Q_ENUM(ExtensionTypes)

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

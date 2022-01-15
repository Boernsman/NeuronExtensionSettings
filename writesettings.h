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

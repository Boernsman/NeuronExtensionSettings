#include <QCoreApplication>
#include <QtSerialBus>
#include <QDebug>
#include <QSerialPort>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    enum Baudrate {
        Baurate_2400 = 11,
        Baurate_4800 = 12,
        Baurate_9600 = 13,
        Baurate_19200 = 14,
        Baurate_38400 = 15,
        Baurate_57600 = 4097,
        Baudrate_115200 = 4098
    };
    uint16_t valueReg0 = 0x0000; //Register 1027
    uint16_t valueReg1 = 0x0000; //Register 1028

    Baudrate baud = Baudrate_115200;
    valueReg0 = baud; //first 12 bits

    bool parityEnabled = true;
    if (parityEnabled) {
        valueReg0 |= (1 << 13);
        bool parityOdd = false;
        if (parityOdd) {
            valueReg0 |= (1 << 14);
        }
    }
    valueReg0 |= (1 << 15);

    uint address = 3; //TODO
    valueReg1 = address;
    QModbusRtuSerialMaster *master = new QModbusRtuSerialMaster();
    master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, "/dev/ttyNS1");
    master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, 19200);
    master->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::Parity::NoParity);
    //m_modbusTcpClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_hostAddress.toString());
    master->setTimeout(200);
    master->setNumberOfRetries(3);

    if (!master->connectDevice()) {
        qWarning() << "Connecing to modbus RTU master failed";
        return -1;
    }


    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1027, 2);
    request.setValue(0, valueReg0);
    request.setValue(1, valueReg1);

    if (QModbusReply *reply = master->sendWriteRequest(request, 15)) {
        if (!reply->isFinished()) {
            QObject::connect(reply, &QModbusReply::finished,[=] {
                if (reply->error() == QModbusDevice::NoError) {
                    qDebug() << "Success 1";
                    QModbusDataUnit request2 = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1003, 1);
                    request2.setValue(0, 0x0001);
                    if (QModbusReply *reply = master->sendWriteRequest(request, 15)) {
                        if (!reply->isFinished()) {
                            QObject::connect(reply, &QModbusReply::finished,[=] {
                                if (reply->error() == QModbusDevice::NoError) {
                                    qDebug() << "Success 2";
                                    QModbusDataUnit request3= QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1002, 1);
                                    if (QModbusReply *reply = master->sendReadRequest(request, 15)) {
                                        if (reply->error() == QModbusDevice::NoError) {
                                            qDebug() << "Success 3";
                                        }
                                    }
                                } else {
                                    qDebug() << "Error" << reply->errorString();
                                }
                            });
                        }
                    }

                } else {
                    qDebug() << "Error" << reply->errorString();
                }
            });
        }
    } else {
        qWarning() << "Error: " << master->errorString();
        return -1;
    }

    QModbusDataUnit request2 = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1005, 2);

    return a.exec();
}

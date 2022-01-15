#include "writesettings.h"
#include <QSerialPort>
#include <QDebug>

WriteSettings::WriteSettings(const QString &serialPort, QObject *parent) : QObject(parent)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
    m_master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, 19200);
    m_master->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::Parity::NoParity);
    m_master->setTimeout(200);
    m_master->setNumberOfRetries(3);
}

void WriteSettings::write(uint address, WriteSettings::Baudrate baud, WriteSettings::Parity parity)
{
    if (address < 1 || address > 255) {
        qWarning() << "Write settings: Address not valid" << address;
        return;
    }
    uint16_t valueReg0 = 0x0000; //Register 1027
    uint16_t valueReg1 = 0x0000; //Register 1028

    valueReg0 = baud; //first 12 bits

    if (parity != Parity::ParityNone) {
        valueReg0 |= (1 << 13);
        if (parity == ParityOdd) {
            valueReg0 |= (1 << 14);
        }
    }
    //valueReg0 |= (1 << 15);
    valueReg1 = address;

    if (!m_master->connectDevice()) {
        qWarning() << "Connecing to modbus RTU master failed";
        return;
    }


    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1027, 2);
    request.setValue(0, valueReg0);
    request.setValue(1, valueReg1);

    if (QModbusReply *reply = m_master->sendWriteRequest(request, 15)) {
        if (!reply->isFinished()) {
            QObject::connect(reply, &QModbusReply::finished,[=] {
                if (reply->error() == QModbusDevice::NoError) {
                    qDebug() << "Writting setting successfull" << reply->result().value(0);
                    QModbusDataUnit request2 = QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1003, 1);
                    request2.setValue(0, 0x0001);
                    if (QModbusReply *reply2 = m_master->sendWriteRequest(request, 15)) {
                        if (!reply2->isFinished()) {
                            QObject::connect(reply2, &QModbusReply::finished,[=] {
                                if (reply2->error() == QModbusDevice::NoError) {
                                    qDebug() << "Storing values successfull" << reply2->result().value(0);
                                    QModbusDataUnit request3= QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, 1003, 1);
                                    if (QModbusReply *reply3 = m_master->sendReadRequest(request, 15)) {
                                        if (reply3->error() == QModbusDevice::NoError) {
                                            qDebug() << "Please power cycle the extension";
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
        qWarning() << "Error: " << m_master->errorString();
        return;
    }
}


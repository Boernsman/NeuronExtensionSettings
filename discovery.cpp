#include "discovery.h"

#include <QDebug>

Discovery::Discovery(const QString &serialPort, uint baudrate, QSerialPort::Parity parity, QObject *parent) : QObject(parent)
{
    m_master = new QModbusRtuSerialMaster(this);
    m_master->setConnectionParameter(QModbusDevice::SerialPortNameParameter, serialPort);
    m_master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrate);
    m_master->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_master->setTimeout(200);
    m_master->setNumberOfRetries(1);
}

bool Discovery::startDiscovery(int startAddress, int endAddress)
{
    qDebug() << "NeuronDiscovery: start Discovery, start Address" << startAddress << "end address" << endAddress;

    if (!m_master->connectDevice()) {
        qDebug() << "Discovery: Could not connect to port";
        return false;
    }

    if (m_discoveryOngoing) {
        qDebug() << "Discovery: Discovery is already in progress";
        return false;
    }
    m_startAddress = startAddress;
    m_endAddress = endAddress;
    getNext(m_startAddress);

    m_sweepingAddress = 1;
    m_discoveryOngoing = true;
    return true;
}

void Discovery::stopDiscovery()
{
    qDebug() << "Discovery: stopping discovery";
    m_sweepingAddress = 1;
    m_discoveryOngoing = false;
}

void Discovery::getNext(int address)
{
    QModbusDataUnit request = QModbusDataUnit(QModbusDataUnit::RegisterType::HoldingRegisters, 1000, 7);
    QModbusReply *reply = m_master->sendReadRequest(request, address);
    connect(reply, &QModbusReply::finished, reply, &QModbusReply::deleteLater);
    connect(reply, &QModbusReply::finished, this, [this, reply] {

        if (reply->serverAddress() == m_sweepingAddress) {
            m_sweepingAddress = reply->serverAddress()+1;
        } else if (reply->serverAddress() < m_sweepingAddress){
            // A reply returns multiple finish signals depending on the retry
            qWarning() << "Discovery: Got modbus reply from previous request, ignoring";
            return;
        }

        QVector<quint16> result = reply->result().values();
        if (result.length() == 7) {
            qDebug() << "Discovery: Found Extension";
            qDebug() << "     - Slave Address" << reply->serverAddress();
            qDebug() << "     - Hardware Id" << result[4];
            qDebug() << "     - Serial number" << (static_cast<quint32>(result[6])<<16 | result[5]);

            ExtensionTypes model;
            if (result[4] == 1) {
                model = ExtensionTypes::xS10;
            } else if (result[4] == 784) {
                model = ExtensionTypes::xS30;
            } else if (result[4] == 528) {
                model = ExtensionTypes::xS40;
            } else if (result[4] == 5) {
                model = ExtensionTypes::xS50;
            } else if (result[4] == 272) {
                model = ExtensionTypes::xS11;
            } else if (result[4] == 273) {
                model = ExtensionTypes::xS51;
            } else {
                qDebug() << "Unkown model" << result[4];
                model = ExtensionTypes::Unknown;
            }
            qDebug() << "     - Model" << model;
        }
        if (reply->serverAddress() >= m_endAddress) {
            m_discoveryOngoing = false;
            qWarning() << "Discovery: Discovery finished";
        } else {
            if (m_discoveryOngoing)
                getNext(m_sweepingAddress);
        }
    });
}

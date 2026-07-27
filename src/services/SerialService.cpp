#include "SerialService.h"

#include <QDebug>

SerialService::SerialService(const QString &portName, QObject *parent)
    : QObject(parent),
      m_portName(portName)
{
    m_serial = new QSerialPort(this);
    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(m_serial, &QSerialPort::readyRead, this, &SerialService::handleReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialService::handleError);

    m_watchdogTimer = new QTimer(this);
    m_watchdogTimer->setInterval(500); // 500ms timeout means stale data
    connect(m_watchdogTimer, &QTimer::timeout, this, &SerialService::handleWatchdogTimeout);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(2000); // Try reconnect every 2s if disconnected
    connect(m_reconnectTimer, &QTimer::timeout, this, &SerialService::tryReconnect);
}

SerialService::~SerialService()
{
    stopService();
}

void SerialService::startService()
{
    if (m_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Serial port" << m_portName << "opened successfully. Waiting for data...";
        m_watchdogTimer->start();
        m_reconnectTimer->stop();
    } else {
        qWarning() << "Failed to open serial port" << m_portName << "-" << m_serial->errorString();
        setConnected(false);
        m_reconnectTimer->start();
    }
}

void SerialService::stopService()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_watchdogTimer->stop();
    m_reconnectTimer->stop();
}

void SerialService::sendCommand(const QString &command)
{
    if (m_serial->isOpen() && m_serial->isWritable()) {
        m_serial->write((command + "\n").toUtf8());
    }
}

void SerialService::emergencyStop()
{
    sendCommand("STOP;");
}

void SerialService::handleReadyRead()
{
    processIncomingBytes(m_serial->readAll());
}

void SerialService::processIncomingBytes(const QByteArray &bytes)
{
    const auto frames = m_parser.append(bytes);
    for (const RawSerialTelemetry &frame : frames) {
        m_watchdogTimer->start();
        setConnected(true);
        emit rawTelemetryUpdated(frame.rpm, frame.batteryVoltage, frame.errorCode);
    }
}

void SerialService::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qWarning() << "Serial port resource error, disconnecting...";
        m_watchdogTimer->stop();
        if (m_serial->isOpen()) {
            m_serial->close();
        }
        m_parser.clear();
        setConnected(false);
        m_reconnectTimer->start();
    }
}

void SerialService::handleWatchdogTimeout()
{
    qWarning() << "Watchdog timeout! No telemetry data received.";
    m_watchdogTimer->stop();
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_parser.clear();
    setConnected(false);
    m_reconnectTimer->start();
}

void SerialService::tryReconnect()
{
    qDebug() << "Attempting to reconnect to" << m_portName << "...";
    if (m_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Reconnected successfully.";
        m_watchdogTimer->start();
        m_reconnectTimer->stop();
    }
}

void SerialService::setConnected(bool connected)
{
    if (m_connectionStateKnown && m_isConnected == connected) {
        return;
    }

    m_connectionStateKnown = true;
    m_isConnected = connected;
    emit connectionStatusChanged(connected);
}

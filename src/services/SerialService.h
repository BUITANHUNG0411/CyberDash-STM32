#pragma once

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QString>
#include <QByteArray>

#include "SerialTelemetryParser.h"

class SerialService : public QObject
{
    Q_OBJECT

public:
    explicit SerialService(const QString &portName = "/dev/ttyUSB0", QObject *parent = nullptr);
    ~SerialService();

    void startService();
    void stopService();
    void sendCommand(const QString &command);
    void emergencyStop();

signals:
    void rawTelemetryUpdated(int rpm, double batteryVoltage, int errorCode);
    void connectionStatusChanged(bool isConnected);

private slots:
    void handleReadyRead();
    void processIncomingBytes(const QByteArray &bytes);
    void handleError(QSerialPort::SerialPortError error);
    void handleWatchdogTimeout();
    void tryReconnect();

private:
    void setConnected(bool connected);

    QSerialPort *m_serial;
    QTimer *m_watchdogTimer;
    QTimer *m_reconnectTimer;
    QString m_portName;
    SerialTelemetryParser m_parser;
    bool m_isConnected = false;
    bool m_connectionStateKnown = false;
};

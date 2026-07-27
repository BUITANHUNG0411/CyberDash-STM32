#include <QtTest>

#include <functional>
#include <limits>

#include "services/SerialService.h"
#include "services/SerialTelemetryParser.h"
#include "services/TelemetryMapper.h"

class ControlledOpenSerialService final : public SerialService
{
public:
    using SerialService::SerialService;

    std::function<void()> beforeOpen;

protected:
    bool openSerialPort(QIODevice::OpenMode mode) override
    {
        Q_UNUSED(mode)
        if (beforeOpen) {
            beforeOpen();
        }
        return true;
    }
};

class TestSerialPipeline : public QObject
{
    Q_OBJECT

private slots:
    void validFrameIsAccepted()
    {
        SerialTelemetryParser parser;
        const auto frames = parser.append("TEL,118,11.8,0;129\n");
        QCOMPARE(frames.size(), 1);
        QCOMPARE(frames.first().rpm, 118);
        QCOMPARE(frames.first().batteryVoltage, 11.8);
        QCOMPARE(frames.first().errorCode, 0);
    }

    void invalidChecksumIsRejected()
    {
        SerialTelemetryParser parser;
        QCOMPARE(parser.append("TEL,118,11.8,0;128\n").size(), 0);
    }

    void partialFrameIsAccumulated()
    {
        SerialTelemetryParser parser;
        QCOMPARE(parser.append("TEL,118,11.").size(), 0);
        QCOMPARE(parser.append("8,0;129\n").size(), 1);
    }

    void oversizedInputIsDiscarded()
    {
        SerialTelemetryParser parser;
        QCOMPARE(parser.append(QByteArray(4097, 'X')).size(), 0);
        QCOMPARE(parser.append("TEL,118,11.8,0;129\n").size(), 1);
    }

    void fullRangeIntegersUseModuloChecksum()
    {
        SerialTelemetryParser parser;
        const auto frames = parser.append("TEL,2147483647,0,1;0\n");
        QCOMPARE(frames.size(), 1);
        QCOMPARE(frames.first().rpm, (std::numeric_limits<int>::max)());
        QCOMPARE(frames.first().errorCode, 1);
    }

    void outOfRangeBatteryIsRejected()
    {
        SerialTelemetryParser parser;
        QCOMPARE(parser.append("TEL,1,2147483648,0;1\n").size(), 0);
    }

    void rawTelemetryMapsOutsideTransport()
    {
        const auto value = TelemetryMapper::fromSerial({3000, 11.8, 0});
        QCOMPARE(value.speed, 90.0);
        QCOMPARE(value.rpm, 3000);
        QCOMPARE(value.gear, QString("5"));
        QCOMPARE(value.warning, false);
        QCOMPARE(value.battery, 100);
        QCOMPARE(value.range, 325);
        QCOMPARE(value.temperature, 57);
    }

    void connectedResourceErrorPublishesDisconnected()
    {
        SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
        QSignalSpy spy(&service, &SerialService::connectionStatusChanged);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(spy.size(), 1);
        QVERIFY(spy.takeFirst().first().toBool());

        QVERIFY(QMetaObject::invokeMethod(
            &service, "handleError", Qt::DirectConnection,
            Q_ARG(QSerialPort::SerialPortError, QSerialPort::ResourceError)));
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), false);
    }

    void connectedWatchdogPublishesDisconnected()
    {
        SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
        QSignalSpy spy(&service, &SerialService::connectionStatusChanged);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(spy.size(), 1);
        QVERIFY(spy.takeFirst().first().toBool());

        QVERIFY(QMetaObject::invokeMethod(&service, "handleWatchdogTimeout",
                                          Qt::DirectConnection));
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().first().toBool(), false);
    }

    void validBytesEmitRawTelemetry()
    {
        SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
        QSignalSpy telemetrySpy(&service, &SerialService::rawTelemetryUpdated);
        QSignalSpy connectionSpy(&service, &SerialService::connectionStatusChanged);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 1);
        const QList<QVariant> arguments = telemetrySpy.takeFirst();
        QCOMPARE(arguments.at(0).toInt(), 118);
        QCOMPARE(arguments.at(1).toDouble(), 11.8);
        QCOMPARE(arguments.at(2).toInt(), 0);
        QCOMPARE(connectionSpy.size(), 1);
        QVERIFY(connectionSpy.takeFirst().at(0).toBool());
    }

    void disconnectClearsPartialFrame()
    {
        SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
        QSignalSpy telemetrySpy(&service, &SerialService::rawTelemetryUpdated);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11."))));
        QVERIFY(QMetaObject::invokeMethod(
            &service, "handleError", Qt::DirectConnection,
            Q_ARG(QSerialPort::SerialPortError, QSerialPort::ResourceError)));
        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 0);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 1);
    }

    void successfulSilentOpenPublishesInitialDisconnectedState()
    {
        ControlledOpenSerialService service;
        QSignalSpy connectionSpy(&service, &SerialService::connectionStatusChanged);
        bool disconnectedBeforeOpen = false;
        service.beforeOpen = [&connectionSpy, &disconnectedBeforeOpen]() {
            disconnectedBeforeOpen =
                connectionSpy.size() == 1
                && !connectionSpy.first().first().toBool();
        };

        service.startService();

        QVERIFY(disconnectedBeforeOpen);
        QCOMPARE(connectionSpy.size(), 1);
        QCOMPARE(connectionSpy.first().first().toBool(), false);
    }

    void successfulReconnectStaysDisconnectedUntilValidFrame()
    {
        ControlledOpenSerialService service;
        QSignalSpy connectionSpy(&service, &SerialService::connectionStatusChanged);

        QVERIFY(QMetaObject::invokeMethod(&service, "tryReconnect",
                                          Qt::DirectConnection));
        QCOMPARE(connectionSpy.size(), 0);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(connectionSpy.size(), 1);
        QVERIFY(connectionSpy.takeFirst().first().toBool());
    }

    void stopClearsPartialFrameAndPublishesDisconnectedState()
    {
        SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
        QSignalSpy telemetrySpy(&service, &SerialService::rawTelemetryUpdated);
        QSignalSpy connectionSpy(&service, &SerialService::connectionStatusChanged);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 1);
        QCOMPARE(connectionSpy.size(), 1);
        QVERIFY(connectionSpy.takeFirst().first().toBool());

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11."))));
        service.stopService();
        QCOMPARE(connectionSpy.size(), 1);
        QCOMPARE(connectionSpy.takeFirst().first().toBool(), false);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 1);

        QVERIFY(QMetaObject::invokeMethod(
            &service, "processIncomingBytes", Qt::DirectConnection,
            Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
        QCOMPARE(telemetrySpy.size(), 2);
    }
};

QTEST_MAIN(TestSerialPipeline)

#include "tst_serial_pipeline.moc"

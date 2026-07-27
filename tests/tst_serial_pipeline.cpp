#include <QtTest>

#include "services/SerialTelemetryParser.h"
#include "services/TelemetryMapper.h"

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
};

QTEST_MAIN(TestSerialPipeline)

#include "tst_serial_pipeline.moc"

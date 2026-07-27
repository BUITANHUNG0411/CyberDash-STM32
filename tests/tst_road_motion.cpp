#include <QtTest>

#include <cmath>

#include "services/MockWheelTelemetryService.h"

class TestRoadMotion final : public QObject
{
    Q_OBJECT

private slots:
    void mockStartsStraight()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(10);

        QCOMPARE(spy.size(), 1);
        const auto sample = spy.takeFirst();
        QCOMPARE(sample.at(0).toDouble(), 1.0);
        QCOMPARE(sample.at(1).toDouble(), 1.0);
        QCOMPARE(sample.at(2).toLongLong(), 10);
    }

    void mockVisitsLeftStraightAndRightStages()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(120);
        auto sample = spy.takeLast();
        QVERIFY(sample.at(1).toDouble() > sample.at(0).toDouble());

        service.advance(100);
        sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), sample.at(1).toDouble());

        service.advance(100);
        sample = spy.takeLast();
        QVERIFY(sample.at(0).toDouble() > sample.at(1).toDouble());
    }

    void mockLoopsWithFiniteSmoothSamples()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        for (int i = 0; i < 50; ++i) {
            service.advance(10);
        }

        QCOMPARE(spy.size(), 50);
        double previousLeft = spy.at(0).at(0).toDouble();
        for (const auto &arguments : spy) {
            const double left = arguments.at(0).toDouble();
            const double right = arguments.at(1).toDouble();
            QVERIFY(std::isfinite(left));
            QVERIFY(std::isfinite(right));
            const double delta = std::abs(left - previousLeft);
            QVERIFY2(delta <= 0.175 + 1.0e-12,
                     qPrintable(QStringLiteral("left=%1 previous=%2 delta=%3")
                                    .arg(left, 0, 'g', 17)
                                    .arg(previousLeft, 0, 'g', 17)
                                    .arg(delta, 0, 'g', 17)));
            previousLeft = left;
        }
    }
};

QTEST_MAIN(TestRoadMotion)

#include "tst_road_motion.moc"

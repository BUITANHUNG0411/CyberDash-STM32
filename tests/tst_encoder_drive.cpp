#include <QtTest>

#include <cmath>
#include <limits>

#include "services/MockWheelTelemetryService.h"

class TestEncoderDrive final : public QObject
{
    Q_OBJECT

private slots:
    void mockStartsStraight()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(20);

        QCOMPARE(spy.size(), 1);
        const auto sample = spy.takeFirst();
        QCOMPARE(sample.at(0).toDouble(), 1.0);
        QCOMPARE(sample.at(1).toDouble(), 1.0);
        QCOMPARE(sample.at(2).toLongLong(), 20);
    }

    void mockVisitsGentleAndStrongStages()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        // Advance into gentle-left: right > left, and the difference is <= 20%.
        service.advance(100);
        service.advance(20);
        const auto gentleLeft = spy.takeLast();
        const double gentleLeftMotion = gentleLeft.at(0).toDouble();
        const double gentleRightMotion = gentleLeft.at(1).toDouble();
        QVERIFY(gentleRightMotion > gentleLeftMotion);
        QVERIFY((gentleRightMotion - gentleLeftMotion) / gentleRightMotion <= 0.20);

        // Advance into strong-left: right > left and the difference is > 20%.
        service.advance(100);
        service.advance(100);
        service.advance(100);
        service.advance(20);
        const auto strongLeft = spy.takeLast();
        const double strongLeftMotion = strongLeft.at(0).toDouble();
        const double strongRightMotion = strongLeft.at(1).toDouble();
        QVERIFY(strongRightMotion > strongLeftMotion);
        QVERIFY((strongRightMotion - strongLeftMotion) / strongRightMotion > 0.20);
    }

    void mockLoopsSevenStages()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        QVERIFY(config.targets.size() == 7U);

        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        const std::array<WheelMotionTarget, 7> expectedTargets{{
            {1.00, 1.00},
            {0.90, 1.00},
            {1.00, 1.00},
            {1.00, 0.90},
            {0.55, 1.00},
            {1.00, 1.00},
            {1.00, 0.55}
        }};

        service.advance(20);
        for (std::size_t stage = 1; stage < expectedTargets.size(); ++stage) {
            service.advance(80);
            service.advance(20);
        }

        QCOMPARE(spy.size(), 13);
        for (std::size_t stage = 0; stage < expectedTargets.size(); ++stage) {
            const auto sample = spy.at(static_cast<qsizetype>(stage * 2U));
            const double left = sample.at(0).toDouble();
            const double right = sample.at(1).toDouble();
            QVERIFY(std::isfinite(left));
            QVERIFY(std::isfinite(right));
            QVERIFY(left >= 0.0 && left <= 1.0);
            QVERIFY(right >= 0.0 && right <= 1.0);
            QCOMPARE(left, expectedTargets.at(stage).left);
            QCOMPARE(right, expectedTargets.at(stage).right);
        }
    }

    void mockClampsOversizedElapsedTime()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(1000);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);

        service.advance((std::numeric_limits<qint64>::max)());
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);
    }

    void mockUsesInjectedSevenTargets()
    {
        MockWheelTelemetryConfig config;
        QVERIFY(config.targets.size() == 7U);
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        config.targets[1] = {0.80, 0.90};
        config.targets[4] = {0.20, 0.80};
        config.targets[6] = {0.70, 0.10};
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(100);
        service.advance(20);
        auto sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), 0.80);
        QCOMPARE(sample.at(1).toDouble(), 0.90);

        service.advance(100);
        service.advance(100);
        service.advance(100);
        service.advance(20);
        sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), 0.20);
        QCOMPARE(sample.at(1).toDouble(), 0.80);

        service.advance(100);
        service.advance(100);
        service.advance(20);
        sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), 0.70);
        QCOMPARE(sample.at(1).toDouble(), 0.10);
    }

    void mockClampsTimerIntervalToAcceptedMaximum()
    {
        MockWheelTelemetryConfig config;
        config.timerInterval = std::chrono::milliseconds{1000};
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.start();
        QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 250);
        service.stop();

        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);
    }
};

QTEST_MAIN(TestEncoderDrive)
#include "tst_encoder_drive.moc"

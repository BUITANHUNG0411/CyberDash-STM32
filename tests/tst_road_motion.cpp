#include <QtTest>

#include <cmath>
#include <limits>

#include "services/MockWheelTelemetryService.h"
#include "viewmodels/RoadMotionViewModel.h"

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

        service.advance(100);
        service.advance(20);
        auto sample = spy.takeLast();
        QVERIFY(sample.at(1).toDouble() > sample.at(0).toDouble());

        service.advance(80);
        service.advance(20);
        sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), sample.at(1).toDouble());

        service.advance(80);
        service.advance(20);
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

    void mockClampsOversizedElapsedTime()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(1000);

        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);

        service.advance((std::numeric_limits<qint64>::max)());
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);
    }

    void mockUsesInjectedWheelTargets()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        config.targets[1] = {0.20, 0.80};
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(100);
        service.advance(20);

        const auto sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), 0.20);
        QCOMPARE(sample.at(1).toDouble(), 0.80);
    }

    void mockClampsTimerIntervalToAcceptedMaximum()
    {
        MockWheelTelemetryConfig config;
        config.timerInterval = std::chrono::milliseconds{1000};
        MockWheelTelemetryService service(config);
        QSignalSpy spy(
            &service,
            &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.start();
        QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 250);
        service.stop();

        QCOMPARE(spy.takeFirst().at(2).toLongLong(), 100);
    }

    void roadModelHasStableFiniteRows()
    {
        RoadMotionViewModel model;

        QCOMPARE(model.rowCount(), 24);
        for (int row = 0; row < model.rowCount(); ++row) {
            const QModelIndex index = model.index(row, 0);
            QVERIFY(std::isfinite(
                model.data(index, RoadMotionViewModel::NearYRole).toDouble()));
            QVERIFY(std::isfinite(
                model.data(index, RoadMotionViewModel::FarYRole).toDouble()));
            QVERIFY(std::isfinite(
                model.data(index, RoadMotionViewModel::LeftNearXRole).toDouble()));
            QVERIFY(std::isfinite(
                model.data(index, RoadMotionViewModel::RightNearXRole).toDouble()));
        }
    }

    void wheelDifferenceControlsApprovedTurnSign()
    {
        RoadMotionViewModel model;

        model.updateWheelMotion(0.65, 1.0, 100);
        QVERIFY(model.curvature() > 0.0);

        model.resetRoad();
        model.updateWheelMotion(1.0, 0.65, 100);
        QVERIFY(model.curvature() < 0.0);
    }

    void turnCarriesLateralRoadOffsetIntoStraight()
    {
        RoadMotionViewModel model;

        model.updateWheelMotion(0.65, 1.0, 100);
        for (int i = 0; i < 100; ++i) {
            model.updateWheelMotion(1.0, 1.0, 100);
        }

        const double farCenter = model.data(
            model.index(23, 0),
            RoadMotionViewModel::CenterFarXRole).toDouble();
        QVERIFY(farCenter < 0.499);
    }

    void equalOrStoppedWheelsDoNotCurve()
    {
        RoadMotionViewModel model;

        model.updateWheelMotion(1.0, 1.0, 100);
        QCOMPARE(model.curvature(), 0.0);
        const double depth = model.data(
            model.index(0, 0),
            RoadMotionViewModel::SegmentDepthRole).toDouble();

        model.updateWheelMotion(0.0, 0.0, 100);
        QCOMPARE(model.forwardSpeed(), 0.0);
        QCOMPARE(model.data(
                     model.index(0, 0),
                     RoadMotionViewModel::SegmentDepthRole).toDouble(),
                 depth);
    }

    void unchangedStoppedMotionDoesNotReemitProperties()
    {
        RoadMotionViewModel model;
        QSignalSpy speedSpy(&model, &RoadMotionViewModel::forwardSpeedChanged);
        QSignalSpy curvatureSpy(&model, &RoadMotionViewModel::curvatureChanged);
        QSignalSpy dataSpy(&model, &QAbstractItemModel::dataChanged);

        model.updateWheelMotion(0.0, 0.0, 100);
        model.updateWheelMotion(0.0, 0.0, 100);

        QCOMPARE(speedSpy.size(), 0);
        QCOMPARE(curvatureSpy.size(), 0);
        QCOMPARE(dataSpy.size(), 0);
    }

    void invalidSamplesRemainFiniteAndBounded()
    {
        RoadMotionViewModel model;
        const double invalid[] = {
            -1.0,
            (std::numeric_limits<double>::infinity)(),
            std::numeric_limits<double>::quiet_NaN()
        };

        for (double value : invalid) {
            model.updateWheelMotion(value, value, 1000);
            QVERIFY(std::isfinite(model.forwardSpeed()));
            QVERIFY(std::isfinite(model.curvature()));
            QVERIFY(std::abs(model.curvature()) <= 0.75);
        }
    }

    void subthresholdSpeedIsTreatedAsStopped()
    {
        RoadMotionViewModel model;
        const double depth = model.data(
            model.index(0, 0),
            RoadMotionViewModel::SegmentDepthRole).toDouble();

        model.updateWheelMotion(0.01, 0.01, 100);

        QCOMPARE(model.forwardSpeed(), 0.0);
        QCOMPARE(model.data(
                     model.index(0, 0),
                     RoadMotionViewModel::SegmentDepthRole).toDouble(),
                 depth);
    }

    void excessiveFiniteWheelSpeedsAreClamped()
    {
        RoadMotionViewModel model;

        model.updateWheelMotion(1000.0, 1000.0, 100);

        QVERIFY(std::isfinite(model.forwardSpeed()));
        QVERIFY(model.forwardSpeed() <= 1.0);

        model.updateWheelMotion(
            (std::numeric_limits<double>::max)(),
            (std::numeric_limits<double>::max)(),
            100);
        QVERIFY(std::isfinite(model.forwardSpeed()));
        QVERIFY(model.forwardSpeed() <= 1.0);

        for (int row = 0; row < model.rowCount(); ++row) {
            const QModelIndex index = model.index(row, 0);
            QVERIFY(std::isfinite(
                model.data(index, RoadMotionViewModel::SegmentDepthRole)
                    .toDouble()));
        }
    }

    void rowsRecycleWithoutChangingModelSize()
    {
        RoadMotionViewModel model;
        const int initialRows = model.rowCount();
        bool wrapped = false;
        double previousDepth = model.data(
            model.index(23, 0),
            RoadMotionViewModel::SegmentDepthRole).toDouble();

        for (int i = 0; i < 100; ++i) {
            model.updateWheelMotion(1.0, 1.0, 100);
            const double depth = model.data(
                model.index(23, 0),
                RoadMotionViewModel::SegmentDepthRole).toDouble();
            wrapped = wrapped || depth < previousDepth;
            previousDepth = depth;
        }

        QCOMPARE(model.rowCount(), initialRows);
        QVERIFY(wrapped);
    }

    void staleMotionStopsWithoutResettingRows()
    {
        RoadMotionViewModel model;
        model.updateWheelMotion(1.0, 1.0, 100);
        const double depth = model.data(
            model.index(0, 0),
            RoadMotionViewModel::SegmentDepthRole).toDouble();

        QVERIFY(QMetaObject::invokeMethod(
            &model, "handleStaleTimeout", Qt::DirectConnection));

        QCOMPARE(model.forwardSpeed(), 0.0);
        QCOMPARE(model.data(
                     model.index(0, 0),
                     RoadMotionViewModel::SegmentDepthRole).toDouble(),
                 depth);
    }
};

QTEST_MAIN(TestRoadMotion)

#include "tst_road_motion.moc"

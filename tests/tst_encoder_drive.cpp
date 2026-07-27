#include <QtTest>

#include <cmath>
#include <limits>

#include <QRegularExpression>

#include "services/MockWheelTelemetryService.h"
#include "viewmodels/EncoderDriveViewModel.h"

namespace {
void verifyFiniteNormalizedPath(const QString &path)
{
    QVERIFY(!path.isEmpty());
    QVERIFY(!path.contains(QStringLiteral("nan"), Qt::CaseInsensitive));
    QVERIFY(!path.contains(QStringLiteral("inf"), Qt::CaseInsensitive));

    const QRegularExpression coordinatePattern(
        QStringLiteral(R"((-?\d+\.\d+))"));
    QRegularExpressionMatchIterator matches =
        coordinatePattern.globalMatch(path);
    int coordinateCount = 0;
    while (matches.hasNext()) {
        const double coordinate = matches.next().captured(1).toDouble();
        QVERIFY(std::isfinite(coordinate));
        QVERIFY(coordinate >= 0.0);
        QVERIFY(coordinate <= 1.0);
        ++coordinateCount;
    }
    QVERIFY(coordinateCount >= 8);
}
}

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

    void exactThresholdBands()
    {
        EncoderDriveViewModel model;
        QSignalSpy turnStateSpy(
            &model, &EncoderDriveViewModel::turnStateChanged);
        model.updateWheelMotion(0.90, 0.94, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        QCOMPARE(turnStateSpy.size(), 0);

        model.updateWheelMotion(0.90, 0.96, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::GentleLeft);
        QCOMPARE(turnStateSpy.size(), 1);

        model.updateWheelMotion(0.55, 1.00, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::TurningLeft);
        QCOMPARE(turnStateSpy.size(), 2);

        constexpr double leftJustBelowFivePercent =
            (2.0 - 0.049999) / (2.0 + 0.049999);
        model.updateWheelMotion(leftJustBelowFivePercent, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        QCOMPARE(turnStateSpy.size(), 3);

        constexpr double leftInsideFivePercentTolerance =
            (2.0 - (0.05 - 5.0e-13))
            / (2.0 + (0.05 - 5.0e-13));
        model.updateWheelMotion(leftInsideFivePercentTolerance, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        QCOMPARE(turnStateSpy.size(), 3);

        const double leftAtFivePercent =
            std::nextafter(1.95 / 2.05, 0.0);
        model.updateWheelMotion(leftAtFivePercent, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::GentleLeft);
        QCOMPARE(turnStateSpy.size(), 4);

        const double leftAtTwentyPercent =
            std::nextafter(1.8 / 2.2, 1.0);
        model.updateWheelMotion(leftAtTwentyPercent, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::GentleLeft);
        QCOMPARE(turnStateSpy.size(), 4);

        constexpr double leftJustAboveTwentyPercent =
            (2.0 - 0.200001) / (2.0 + 0.200001);
        model.updateWheelMotion(leftJustAboveTwentyPercent, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::TurningLeft);
        QCOMPARE(turnStateSpy.size(), 5);

        constexpr double leftInsideTwentyPercentTolerance =
            (2.0 - (0.20 + 5.0e-13))
            / (2.0 + (0.20 + 5.0e-13));
        model.updateWheelMotion(leftInsideTwentyPercentTolerance, 1.0, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::TurningLeft);
        QCOMPARE(turnStateSpy.size(), 5);
    }

    void smallDifferenceMovesVehicleAndRoad()
    {
        EncoderDriveViewModel model;
        QSignalSpy offsetSpy(
            &model, &EncoderDriveViewModel::vehicleLateralOffsetChanged);
        QSignalSpy yawSpy(
            &model, &EncoderDriveViewModel::vehicleYawDegreesChanged);
        QSignalSpy curvatureSpy(
            &model, &EncoderDriveViewModel::roadCurvatureChanged);
        model.updateWheelMotion(0.90, 1.00, 100);
        QVERIFY(model.vehicleLateralOffset() < 0.0);
        QVERIFY(model.vehicleYawDegrees() < 0.0);
        QVERIFY(model.roadCurvature() < 0.0);
        QCOMPARE(offsetSpy.size(), 1);
        QCOMPARE(yawSpy.size(), 1);
        QCOMPARE(curvatureSpy.size(), 1);
    }

    void oppositeWheelDifferenceTurnsRight()
    {
        EncoderDriveViewModel model;
        model.updateWheelMotion(1.00, 0.55, 100);

        QCOMPARE(model.turnState(), EncoderDriveViewModel::TurningRight);
        QVERIFY(model.vehicleLateralOffset() > 0.0);
        QVERIFY(model.vehicleYawDegrees() > 0.0);
        QVERIFY(model.roadCurvature() > 0.0);
    }

    void sustainedTurnReturnsSmoothlyToStraight()
    {
        EncoderDriveViewModel model;
        for (int sample = 0; sample < 5; ++sample) {
            model.updateWheelMotion(0.55, 1.00, 100);
        }

        const qreal turnOffset = model.vehicleLateralOffset();
        const qreal turnYaw = model.vehicleYawDegrees();
        const qreal turnCurvature = model.roadCurvature();
        QVERIFY(turnOffset < 0.0);
        QVERIFY(turnYaw < 0.0);
        QVERIFY(turnCurvature < 0.0);

        model.updateWheelMotion(1.00, 1.00, 50);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        QVERIFY(model.vehicleLateralOffset() > turnOffset);
        QVERIFY(model.vehicleLateralOffset() < 0.0);
        QVERIFY(model.vehicleYawDegrees() > turnYaw);
        QVERIFY(model.vehicleYawDegrees() < 0.0);
        QVERIFY(model.roadCurvature() > turnCurvature);
        QVERIFY(model.roadCurvature() < 0.0);
    }

    void pathsAreFiniteAndChangeOnlyWhenNeeded()
    {
        EncoderDriveViewModel model;
        verifyFiniteNormalizedPath(model.roadPath());
        verifyFiniteNormalizedPath(model.roadEdgePath());

        QSignalSpy roadPathSpy(&model, &EncoderDriveViewModel::roadPathChanged);
        QSignalSpy edgePathSpy(
            &model, &EncoderDriveViewModel::roadEdgePathChanged);

        model.updateWheelMotion(0.55, 1.00, 100);
        QCOMPARE(roadPathSpy.size(), 1);
        QCOMPARE(edgePathSpy.size(), 1);
        verifyFiniteNormalizedPath(model.roadPath());
        verifyFiniteNormalizedPath(model.roadEdgePath());

        EncoderDriveViewModel stoppedModel;
        QSignalSpy stoppedRoadSpy(
            &stoppedModel, &EncoderDriveViewModel::roadPathChanged);
        QSignalSpy stoppedEdgeSpy(
            &stoppedModel, &EncoderDriveViewModel::roadEdgePathChanged);
        QSignalSpy stoppedSpeedSpy(
            &stoppedModel, &EncoderDriveViewModel::forwardSpeedChanged);
        stoppedModel.updateWheelMotion(0.0, 0.0, 100);
        stoppedModel.updateWheelMotion(0.0, 0.0, 100);
        QCOMPARE(stoppedRoadSpy.size(), 0);
        QCOMPARE(stoppedEdgeSpy.size(), 0);
        QCOMPARE(stoppedSpeedSpy.size(), 0);
    }

    void invalidAndExtremeSamplesStayBounded()
    {
        EncoderDriveViewModel model;
        model.updateWheelMotion(
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::infinity(),
            100);

        QCOMPARE(model.forwardSpeed(), 0.0);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);

        model.updateWheelMotion(-1000.0, 1000.0, 100);
        QVERIFY(std::isfinite(model.forwardSpeed()));
        QVERIFY(model.forwardSpeed() >= 0.0);
        QVERIFY(model.forwardSpeed() <= 1.0);
        QVERIFY(std::isfinite(model.vehicleLateralOffset()));
        QVERIFY(std::abs(model.vehicleLateralOffset()) <= 1.0);
        QVERIFY(std::isfinite(model.vehicleYawDegrees()));
        QVERIFY(std::abs(model.vehicleYawDegrees()) <= 20.0);
        QVERIFY(std::isfinite(model.roadCurvature()));
        QVERIFY(std::abs(model.roadCurvature()) <= 1.0);
        verifyFiniteNormalizedPath(model.roadPath());
        verifyFiniteNormalizedPath(model.roadEdgePath());
    }

    void oversizedInputsClampIndependently()
    {
        EncoderDriveViewModel model;
        model.updateWheelMotion(2.0, 1.0, 100);

        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        QCOMPARE(model.forwardSpeed(), 1.0);
        QCOMPARE(model.vehicleLateralOffset(), 0.0);
        QCOMPARE(model.vehicleYawDegrees(), 0.0);
        QCOMPARE(model.roadCurvature(), 0.0);
    }

    void staleInputStopsWithoutResettingOffset()
    {
        EncoderDriveViewModel model;
        model.updateWheelMotion(0.55, 1.00, 100);
        const qreal offset = model.vehicleLateralOffset();
        const qreal speed = model.forwardSpeed();
        const qreal yaw = model.vehicleYawDegrees();
        const qreal curvature = model.roadCurvature();
        QVERIFY(offset < 0.0);

        QVERIFY(QMetaObject::invokeMethod(
            &model, "handleStaleTimeout", Qt::DirectConnection));

        QVERIFY(model.forwardSpeed() < speed);
        QVERIFY(model.forwardSpeed() > 0.0);
        QCOMPARE(model.vehicleLateralOffset(), offset);
        QVERIFY(model.vehicleYawDegrees() < 0.0);
        QVERIFY(model.vehicleYawDegrees() > yaw);
        QVERIFY(model.roadCurvature() < 0.0);
        QVERIFY(model.roadCurvature() > curvature);
        QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);
        verifyFiniteNormalizedPath(model.roadPath());
        verifyFiniteNormalizedPath(model.roadEdgePath());

        const qreal decayedSpeed = model.forwardSpeed();
        QVERIFY(QMetaObject::invokeMethod(
            &model, "handleStaleTimeout", Qt::DirectConnection));
        QVERIFY(model.forwardSpeed() < decayedSpeed);
        QCOMPARE(model.vehicleLateralOffset(), offset);
    }

    void elapsedTimeIsCapped()
    {
        EncoderDriveViewModel cappedModel;
        EncoderDriveViewModel oversizedModel;
        EncoderDriveViewModel rejectedModel;
        const QString initialRoadPath = rejectedModel.roadPath();
        QSignalSpy rejectedSpeedSpy(
            &rejectedModel, &EncoderDriveViewModel::forwardSpeedChanged);

        cappedModel.updateWheelMotion(0.55, 1.00, 100);
        oversizedModel.updateWheelMotion(
            0.55, 1.00, (std::numeric_limits<qint64>::max)());
        rejectedModel.updateWheelMotion(0.55, 1.00, 0);
        rejectedModel.updateWheelMotion(0.55, 1.00, -1);

        QCOMPARE(oversizedModel.forwardSpeed(), cappedModel.forwardSpeed());
        QCOMPARE(oversizedModel.vehicleLateralOffset(),
                 cappedModel.vehicleLateralOffset());
        QCOMPARE(oversizedModel.vehicleYawDegrees(),
                 cappedModel.vehicleYawDegrees());
        QCOMPARE(oversizedModel.roadCurvature(),
                 cappedModel.roadCurvature());
        QCOMPARE(oversizedModel.roadPath(), cappedModel.roadPath());
        QCOMPARE(oversizedModel.roadEdgePath(), cappedModel.roadEdgePath());
        QCOMPARE(rejectedModel.forwardSpeed(), 0.0);
        QCOMPARE(rejectedModel.turnState(), EncoderDriveViewModel::Straight);
        QCOMPARE(rejectedModel.roadPath(), initialRoadPath);
        QCOMPARE(rejectedSpeedSpy.size(), 0);
    }
};

QTEST_MAIN(TestEncoderDrive)
#include "tst_encoder_drive.moc"

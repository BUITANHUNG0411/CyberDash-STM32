#include <QtTest>

#include <cmath>
#include <limits>

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QSignalSpy>

#include "services/MockPositionSource.h"
#include "viewmodels/MapViewModel.h"

namespace {
QGeoPositionInfo positionInfo(const QGeoCoordinate &coordinate,
                              qreal direction)
{
    QGeoPositionInfo info(coordinate, QDateTime::currentDateTimeUtc());
    info.setAttribute(QGeoPositionInfo::Direction, direction);
    return info;
}

class InjectedPositionSource final : public QGeoPositionInfoSource
{
public:
    explicit InjectedPositionSource(QObject *parent = nullptr)
        : QGeoPositionInfoSource(parent)
    {
    }

    PositioningMethods supportedPositioningMethods() const override
    {
        return AllPositioningMethods;
    }

    int minimumUpdateInterval() const override
    {
        return 0;
    }

    Error error() const override
    {
        return NoError;
    }

    QGeoPositionInfo lastKnownPosition(
        bool fromSatellitePositioningMethodsOnly = false) const override
    {
        Q_UNUSED(fromSatellitePositioningMethodsOnly)
        return m_lastPosition;
    }

    void publish(const QGeoPositionInfo &info)
    {
        m_lastPosition = info;
        emit positionUpdated(info);
    }

public:
    void startUpdates() override {}
    void stopUpdates() override {}
    void requestUpdate(int timeout = 0) override
    {
        Q_UNUSED(timeout)
    }

private:
    QGeoPositionInfo m_lastPosition;
};
}

class TestMapNavigation final : public QObject
{
    Q_OBJECT

private slots:
    void sourceInterpolatesWithinSegment()
    {
        MockPositionConfig config;
        config.route = {
            QGeoCoordinate(10.0, 106.0),
            QGeoCoordinate(10.0, 106.001),
            QGeoCoordinate(10.001, 106.001),
            QGeoCoordinate(10.001, 106.0),
        };
        config.speedMetersPerSecond = 10.0;
        config.maximumAdvanceMs = 1000;
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);

        source.advance(100);

        QCOMPARE(spy.size(), 1);
        const QGeoPositionInfo fix = qvariant_cast<QGeoPositionInfo>(
            spy.takeFirst().at(0));
        const qreal expectedDirection = static_cast<qreal>(
            config.route.at(0).azimuthTo(config.route.at(1)));
        const QGeoCoordinate expected = config.route.at(0).atDistanceAndAzimuth(
            1.0, expectedDirection);
        QVERIFY(fix.coordinate().distanceTo(expected) < 0.01);
    }

    void sourceBearingMatchesActiveSegment()
    {
        MockPositionConfig config;
        config.route = {
            QGeoCoordinate(10.0, 106.0),
            QGeoCoordinate(10.0, 106.001),
            QGeoCoordinate(10.001, 106.001),
            QGeoCoordinate(10.001, 106.0),
        };
        config.speedMetersPerSecond = 10.0;
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);

        source.advance(100);

        QCOMPARE(spy.size(), 1);
        const QGeoPositionInfo fix = qvariant_cast<QGeoPositionInfo>(
            spy.takeFirst().at(0));
        const qreal expected = static_cast<qreal>(
            config.route.at(0).azimuthTo(config.route.at(1)));
        QVERIFY(std::abs(fix.attribute(QGeoPositionInfo::Direction) - expected)
                < 0.001);
    }

    void sourceUsesNextSegmentBearingAfterTurn()
    {
        MockPositionConfig config;
        config.route = {
            QGeoCoordinate(10.0, 106.0),
            QGeoCoordinate(10.0, 106.00001),
            QGeoCoordinate(10.00001, 106.00001),
            QGeoCoordinate(10.00001, 106.0),
        };
        config.speedMetersPerSecond = 1000.0;
        config.maximumAdvanceMs = 1000;
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);

        source.advance(2);

        QCOMPARE(spy.size(), 1);
        const QGeoPositionInfo fix = qvariant_cast<QGeoPositionInfo>(
            spy.takeFirst().at(0));
        const qreal expected = static_cast<qreal>(
            config.route.at(1).azimuthTo(config.route.at(2)));
        QVERIFY(std::abs(fix.attribute(QGeoPositionInfo::Direction) - expected)
                < 0.001);
    }

    void sourceCapsOversizedElapsedAdvance()
    {
        MockPositionConfig config;
        config.speedMetersPerSecond = 10.0;
        config.maximumAdvanceMs = 100;
        MockPositionSource cappedSource(config);
        MockPositionSource oversizedSource(config);
        QSignalSpy cappedSpy(&cappedSource,
                             &MockPositionSource::positionUpdated);
        QSignalSpy oversizedSpy(&oversizedSource,
                                &MockPositionSource::positionUpdated);

        cappedSource.advance(100);
        oversizedSource.advance(10000);

        QCOMPARE(cappedSpy.size(), 1);
        QCOMPARE(oversizedSpy.size(), 1);
        const QGeoPositionInfo capped = qvariant_cast<QGeoPositionInfo>(
            cappedSpy.takeFirst().at(0));
        const QGeoPositionInfo oversized = qvariant_cast<QGeoPositionInfo>(
            oversizedSpy.takeFirst().at(0));
        QVERIFY(capped.coordinate().distanceTo(oversized.coordinate()) < 0.01);
        QCOMPARE(capped.timestamp(), oversized.timestamp());
    }

    void sourceWrapsAfterCompleteRoute()
    {
        MockPositionConfig config;
        config.route = {
            QGeoCoordinate(10.0, 106.0),
            QGeoCoordinate(10.0, 106.00001),
            QGeoCoordinate(10.00001, 106.00001),
            QGeoCoordinate(10.00001, 106.0),
        };
        config.speedMetersPerSecond = 1000.0;
        config.maximumAdvanceMs = 1000;
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);

        source.advance(1000);

        QCOMPARE(spy.size(), 1);
        const QGeoPositionInfo fix = qvariant_cast<QGeoPositionInfo>(
            spy.takeFirst().at(0));
        QVERIFY(fix.coordinate().isValid());
        QVERIFY(std::isfinite(fix.attribute(QGeoPositionInfo::Direction)));
    }

    void sourceRejectsInvalidRouteAndElapsedTime()
    {
        MockPositionConfig config;
        config.route = {QGeoCoordinate(), QGeoCoordinate(10.0, 106.0)};
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);
        QSignalSpy errorSpy(&source, &MockPositionSource::errorOccurred);

        source.advance(0);
        source.advance(-1);
        source.advance(100);
        source.requestUpdate();

        QCOMPARE(spy.size(), 0);
        QCOMPARE(errorSpy.size(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).value<QGeoPositionInfoSource::Error>(),
                 QGeoPositionInfoSource::UnknownSourceError);
    }

    void sourceLifecycleIsIdempotent()
    {
        MockPositionConfig config;
        config.tickIntervalMs = 100;
        config.speedMetersPerSecond = 10.0;
        config.route = {
            QGeoCoordinate(10.0, 106.0),
            QGeoCoordinate(10.0, 106.001),
            QGeoCoordinate(10.001, 106.001),
            QGeoCoordinate(10.001, 106.0),
        };
        MockPositionSource source(config);
        QSignalSpy spy(&source, &MockPositionSource::positionUpdated);

        source.setUpdateInterval(200);
        source.startUpdates();
        source.startUpdates();
        source.stopUpdates();
        source.stopUpdates();
        source.advance(10);
        source.requestUpdate();

        QCOMPARE(spy.size(), 2);
        const QGeoPositionInfo fix = qvariant_cast<QGeoPositionInfo>(
            spy.takeLast().at(0));
        QVERIFY(std::abs(config.route.at(0).distanceTo(fix.coordinate()) - 2.1)
                < 0.01);
    }

    void viewModelFollowsInjectedSource()
    {
        InjectedPositionSource source;
        MapViewModel viewModel;
        QSignalSpy positionSpy(&viewModel, &MapViewModel::positionChanged);
        QSignalSpy bearingSpy(&viewModel, &MapViewModel::bearingDegreesChanged);
        QSignalSpy viewportSpy(&viewModel, &MapViewModel::viewportCenterChanged);
        viewModel.setPositionSource(&source);

        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 450.0));

        QCOMPARE(viewModel.position(), QGeoCoordinate(10.0, 106.0));
        QCOMPARE(viewModel.viewportCenter(), QGeoCoordinate(10.0, 106.0));
        QCOMPARE(viewModel.bearingDegrees(), 90.0);
        QCOMPARE(positionSpy.size(), 1);
        QCOMPARE(bearingSpy.size(), 1);
        QCOMPARE(viewportSpy.size(), 1);
    }

    void replacingSourceDisconnectsOldSource()
    {
        InjectedPositionSource oldSource;
        InjectedPositionSource replacement;
        MapViewModel viewModel;
        viewModel.setPositionSource(&oldSource);
        oldSource.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 10.0));
        viewModel.setPositionSource(&replacement);

        oldSource.publish(positionInfo(QGeoCoordinate(11.0, 107.0), 20.0));
        QCOMPARE(viewModel.position(), QGeoCoordinate(10.0, 106.0));

        replacement.publish(positionInfo(QGeoCoordinate(12.0, 108.0), 30.0));
        QCOMPARE(viewModel.position(), QGeoCoordinate(12.0, 108.0));
    }

    void panSuspendsFollowAndMovesViewport()
    {
        InjectedPositionSource source;
        MapViewModel viewModel;
        viewModel.setPositionSource(&source);
        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 0.0));
        const QGeoCoordinate initialCenter = viewModel.viewportCenter();

        viewModel.panByPixels(100.0, 0.0, 400.0, 400.0);

        QVERIFY(!viewModel.followEnabled());
        QVERIFY(viewModel.viewportCenter().longitude()
                < initialCenter.longitude());
    }

    void panWrapsLongitudeAtDateline()
    {
        InjectedPositionSource source;
        MapViewModel viewModel;
        viewModel.setPositionSource(&source);
        source.publish(positionInfo(QGeoCoordinate(0.0, 179.0), 0.0));
        const qreal worldPixels = 256.0 * std::pow(2.0, 16.5);

        viewModel.panByPixels(-worldPixels * 2.0 / 360.0,
                              0.0,
                              400.0,
                              400.0);

        QVERIFY(std::abs(viewModel.viewportCenter().longitude() + 179.0)
                < 0.001);
    }

    void panClampsMercatorLatitudeAtBothPoles()
    {
        InjectedPositionSource source;
        MapViewModel viewModel;
        viewModel.setPositionSource(&source);
        source.publish(positionInfo(QGeoCoordinate(0.0, 106.0), 0.0));

        viewModel.panByPixels(0.0, 1.0e12, 400.0, 400.0);
        QVERIFY(std::abs(viewModel.viewportCenter().latitude() - 85.05112878)
                < 0.000001);

        viewModel.panByPixels(0.0, -1.0e12, 400.0, 400.0);
        QVERIFY(std::abs(viewModel.viewportCenter().latitude() + 85.05112878)
                < 0.000001);
    }

    void zoomGesturesClampAndRestartFollowDeadline()
    {
        MapViewModel viewModel(10);

        viewModel.zoomByWheelDelta(100000.0);
        QCOMPARE(viewModel.zoomLevel(), 19.0);
        QVERIFY(!viewModel.followEnabled());
        viewModel.advanceFollowClock(9);
        QVERIFY(!viewModel.followEnabled());
        viewModel.zoomByPinchScale(0.000001);
        QCOMPARE(viewModel.zoomLevel(), 3.0);
        viewModel.advanceFollowClock(9);
        QVERIFY(!viewModel.followEnabled());
        viewModel.advanceFollowClock(1);
        QVERIFY(viewModel.followEnabled());
        QCOMPARE(viewModel.zoomLevel(), 16.5);
    }

    void followResumesAtInjectedTimeout()
    {
        InjectedPositionSource source;
        MapViewModel viewModel(20);
        viewModel.setPositionSource(&source);
        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 0.0));
        viewModel.panByPixels(20.0, 0.0, 400.0, 400.0);

        viewModel.advanceFollowClock(19);
        QVERIFY(!viewModel.followEnabled());
        viewModel.advanceFollowClock(1);
        QVERIFY(viewModel.followEnabled());
        QCOMPARE(viewModel.viewportCenter(), viewModel.position());
        QCOMPARE(viewModel.zoomLevel(), 16.5);
    }

    void invalidFixDoesNotChangeViewModel()
    {
        InjectedPositionSource source;
        MapViewModel viewModel;
        viewModel.setPositionSource(&source);
        QSignalSpy positionSpy(&viewModel, &MapViewModel::positionChanged);
        QSignalSpy bearingSpy(&viewModel, &MapViewModel::bearingDegreesChanged);

        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 10.0));
        positionSpy.clear();
        bearingSpy.clear();
        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0), 10.0));
        QGeoPositionInfo missingDirection(
            QGeoCoordinate(11.0, 107.0), QDateTime::currentDateTimeUtc());
        source.publish(missingDirection);
        source.publish(positionInfo(
            QGeoCoordinate(), std::numeric_limits<qreal>::quiet_NaN()));
        source.publish(positionInfo(QGeoCoordinate(10.0, 106.0),
                                    std::numeric_limits<qreal>::infinity()));

        QCOMPARE(viewModel.position(), QGeoCoordinate(10.0, 106.0));
        QCOMPARE(positionSpy.size(), 0);
        QCOMPARE(bearingSpy.size(), 0);
    }
};

QTEST_GUILESS_MAIN(TestMapNavigation)

#include "tst_map_navigation.moc"

#pragma once

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QTimer>
#include <QVector>

struct MockPositionConfig
{
    QVector<QGeoCoordinate> route;
    qreal speedMetersPerSecond = 8.0;
    qint64 tickIntervalMs = 100;
    qint64 maximumAdvanceMs = 1000;
};

class MockPositionSource final : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    explicit MockPositionSource(const MockPositionConfig &config = {},
                                QObject *parent = nullptr);

    void setUpdateInterval(int msec) override;
    PositioningMethods supportedPositioningMethods() const override;
    int minimumUpdateInterval() const override;
    Error error() const override;
    QGeoPositionInfo lastKnownPosition(
        bool fromSatellitePositioningMethodsOnly = false) const override;

    void advance(qint64 elapsedMs);

public slots:
    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 0) override;

private:
    void emitCurrentPosition();

    MockPositionConfig m_config;
    QVector<qreal> m_segmentLengths;
    qreal m_routeLengthMeters = 0.0;
    qreal m_distanceAlongRouteMeters = 0.0;
    qint64 m_elapsedMilliseconds = 0;
    QGeoPositionInfo m_lastPosition;
    QTimer m_timer;
    Error m_error = NoError;
    bool m_routeIsUsable = false;
};

#include "MockPositionSource.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QDateTime>
#include <QGeoPositionInfo>
#include <QTimeZone>

namespace {
constexpr qreal kDefaultSpeedMetersPerSecond = 8.0;
constexpr qint64 kDefaultTickIntervalMs = 100;
constexpr qint64 kDefaultMaximumAdvanceMs = 1000;
constexpr qreal kMaximumSpeedMetersPerSecond = 10000.0;
constexpr qint64 kTimestampCycleMs = 24 * 60 * 60 * 1000;

QVector<QGeoCoordinate> defaultRoute()
{
    return {
        QGeoCoordinate(10.776889, 106.700806),
        QGeoCoordinate(10.777171, 106.702147),
        QGeoCoordinate(10.776295, 106.702782),
        QGeoCoordinate(10.775989, 106.701359),
    };
}

bool isFiniteCoordinate(const QGeoCoordinate &coordinate)
{
    return coordinate.isValid()
        && std::isfinite(coordinate.latitude())
        && std::isfinite(coordinate.longitude());
}
}

MockPositionSource::MockPositionSource(const MockPositionConfig &config,
                                       QObject *parent)
    : QGeoPositionInfoSource(parent)
    , m_config(config)
{
    if (m_config.route.isEmpty()) {
        m_config.route = defaultRoute();
    }
    if (!std::isfinite(m_config.speedMetersPerSecond)
        || m_config.speedMetersPerSecond <= 0.0) {
        m_config.speedMetersPerSecond = kDefaultSpeedMetersPerSecond;
    }
    m_config.speedMetersPerSecond = (std::min)(
        m_config.speedMetersPerSecond, kMaximumSpeedMetersPerSecond);
    if (m_config.tickIntervalMs <= 0) {
        m_config.tickIntervalMs = kDefaultTickIntervalMs;
    }
    m_config.tickIntervalMs = (std::min)(
        m_config.tickIntervalMs,
        static_cast<qint64>((std::numeric_limits<int>::max)()));
    if (m_config.maximumAdvanceMs <= 0) {
        m_config.maximumAdvanceMs = kDefaultMaximumAdvanceMs;
    }

    setUpdateInterval(static_cast<int>(m_config.tickIntervalMs));
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        advance(m_timer.interval());
    });

    if (m_config.route.size() < 2) {
        m_error = UnknownSourceError;
        return;
    }

    for (const QGeoCoordinate &coordinate : std::as_const(m_config.route)) {
        if (!isFiniteCoordinate(coordinate)) {
            m_error = UnknownSourceError;
            return;
        }
    }

    const qsizetype routeSize = m_config.route.size();
    m_segmentLengths.resize(routeSize);
    for (qsizetype index = 0; index < routeSize; ++index) {
        const QGeoCoordinate &from = m_config.route.at(index);
        const QGeoCoordinate &to = m_config.route.at(
            (index + 1) % routeSize);
        const qreal segmentLength = static_cast<qreal>(from.distanceTo(to));
        if (!std::isfinite(segmentLength) || segmentLength <= 0.0) {
            m_error = UnknownSourceError;
            return;
        }
        m_segmentLengths[index] = segmentLength;
        m_routeLengthMeters += segmentLength;
    }

    m_routeIsUsable = std::isfinite(m_routeLengthMeters)
        && m_routeLengthMeters > 0.0;
    if (!m_routeIsUsable) {
        m_error = UnknownSourceError;
    }
}

QGeoPositionInfoSource::PositioningMethods
MockPositionSource::supportedPositioningMethods() const
{
    return AllPositioningMethods;
}

void MockPositionSource::setUpdateInterval(int msec)
{
    const int boundedInterval = (std::max)(
        minimumUpdateInterval(), msec > 0 ? msec : minimumUpdateInterval());
    QGeoPositionInfoSource::setUpdateInterval(boundedInterval);
    m_timer.setInterval(boundedInterval);
}

int MockPositionSource::minimumUpdateInterval() const
{
    return static_cast<int>(m_config.tickIntervalMs);
}

QGeoPositionInfoSource::Error MockPositionSource::error() const
{
    return m_error;
}

QGeoPositionInfo MockPositionSource::lastKnownPosition(
    bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly)
    return m_lastPosition;
}

void MockPositionSource::advance(qint64 elapsedMs)
{
    if (!m_routeIsUsable || elapsedMs <= 0) {
        return;
    }

    const qint64 acceptedElapsedMs = (std::min)(
        elapsedMs, m_config.maximumAdvanceMs);
    const qreal distanceMeters = m_config.speedMetersPerSecond
        * static_cast<qreal>(acceptedElapsedMs) / 1000.0;
    if (!std::isfinite(distanceMeters) || distanceMeters < 0.0) {
        return;
    }

    m_distanceAlongRouteMeters = std::fmod(
        m_distanceAlongRouteMeters + distanceMeters, m_routeLengthMeters);
    if (m_distanceAlongRouteMeters < 0.0) {
        m_distanceAlongRouteMeters += m_routeLengthMeters;
    }
    m_elapsedMilliseconds = (m_elapsedMilliseconds
        + acceptedElapsedMs % kTimestampCycleMs) % kTimestampCycleMs;
    emitCurrentPosition();
}

void MockPositionSource::startUpdates()
{
    if (!m_routeIsUsable) {
        m_error = UnknownSourceError;
        emit errorOccurred(m_error);
        return;
    }

    m_error = NoError;
    if (!m_timer.isActive()) {
        m_timer.setInterval((std::max)(minimumUpdateInterval(),
                                       updateInterval()));
        m_timer.start();
    }
}

void MockPositionSource::stopUpdates()
{
    if (m_timer.isActive()) {
        m_timer.stop();
    }
}

void MockPositionSource::requestUpdate(int timeout)
{
    m_error = NoError;
    if (!m_routeIsUsable) {
        m_error = UnknownSourceError;
        emit errorOccurred(m_error);
        return;
    }
    if (timeout > 0 && timeout < minimumUpdateInterval()) {
        m_error = UpdateTimeoutError;
        emit errorOccurred(m_error);
        return;
    }
    advance((std::max)(minimumUpdateInterval(), updateInterval()));
}

void MockPositionSource::emitCurrentPosition()
{
    qreal remainingDistance = m_distanceAlongRouteMeters;
    qsizetype segmentIndex = 0;
    const qsizetype segmentCount = m_segmentLengths.size();
    while (segmentIndex + 1 < segmentCount
           && remainingDistance >= m_segmentLengths.at(segmentIndex)) {
        remainingDistance -= m_segmentLengths.at(segmentIndex);
        ++segmentIndex;
    }

    const QGeoCoordinate &from = m_config.route.at(segmentIndex);
    const QGeoCoordinate &to = m_config.route.at(
        (segmentIndex + 1) % m_config.route.size());
    const qreal direction = static_cast<qreal>(from.azimuthTo(to));
    const QGeoCoordinate coordinate = from.atDistanceAndAzimuth(
        remainingDistance, direction);
    if (!isFiniteCoordinate(coordinate) || !std::isfinite(direction)) {
        return;
    }

    QGeoPositionInfo info(coordinate, QDateTime::fromMSecsSinceEpoch(
        m_elapsedMilliseconds, QTimeZone::UTC));
    info.setAttribute(QGeoPositionInfo::Direction, direction);
    m_lastPosition = info;
    emit positionUpdated(info);
}

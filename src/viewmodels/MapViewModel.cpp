#include "MapViewModel.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

namespace {
constexpr qreal kDefaultZoomLevel = 16.5;
constexpr qreal kMinimumZoomLevel = 3.0;
constexpr qreal kMaximumZoomLevel = 19.0;
constexpr qreal kMaximumMercatorLatitude = 85.05112878;
constexpr qreal kTileSizePixels = 256.0;
constexpr qint64 kDefaultFollowTimeoutMs = 4000;

bool isFiniteCoordinate(const QGeoCoordinate &coordinate)
{
    return coordinate.isValid()
        && std::isfinite(coordinate.latitude())
        && std::isfinite(coordinate.longitude());
}

qreal wrappedLongitude(qreal longitude)
{
    qreal wrapped = std::fmod(longitude + 180.0, 360.0);
    if (wrapped < 0.0) {
        wrapped += 360.0;
    }
    return wrapped - 180.0;
}
}

MapViewModel::MapViewModel(qint64 followTimeoutMs, QObject *parent)
    : QObject(parent)
    , m_followTimeoutMs(static_cast<qint64>((std::min)(
          followTimeoutMs > 0 ? followTimeoutMs : kDefaultFollowTimeoutMs,
          static_cast<qint64>((std::numeric_limits<int>::max)()))))
{
    m_followTimer.setSingleShot(true);
    m_followTimer.setInterval(static_cast<int>(m_followTimeoutMs));
    connect(&m_followTimer, &QTimer::timeout, this,
            &MapViewModel::resumeFollow);
}

QGeoCoordinate MapViewModel::position() const
{
    return m_position;
}

qreal MapViewModel::bearingDegrees() const
{
    return m_bearingDegrees;
}

QGeoCoordinate MapViewModel::viewportCenter() const
{
    return m_viewportCenter;
}

qreal MapViewModel::zoomLevel() const
{
    return m_zoomLevel;
}

bool MapViewModel::followEnabled() const
{
    return m_followEnabled;
}

QString MapViewModel::followLabel() const
{
    return m_followEnabled ? QStringLiteral("FOLLOW")
                           : QStringLiteral("EXPLORE");
}

void MapViewModel::setPositionSource(QGeoPositionInfoSource *source)
{
    if (m_positionSource.data() == source) {
        return;
    }

    QObject::disconnect(m_positionConnection);
    QObject::disconnect(m_destructionConnection);
    m_positionSource = source;
    if (source == nullptr) {
        return;
    }

    m_positionConnection = connect(
        source, &QGeoPositionInfoSource::positionUpdated, this,
        [this, source](const QGeoPositionInfo &info) {
            if (m_positionSource.data() == source) {
                handlePositionUpdated(info);
            }
        });
    m_destructionConnection = connect(source, &QObject::destroyed, this,
                                      [this, source]() {
        if (m_positionSource.data() == source) {
            m_positionSource = nullptr;
        }
    });
}

void MapViewModel::panByPixels(qreal dx,
                               qreal dy,
                               qreal viewportWidth,
                               qreal viewportHeight)
{
    if (!isFiniteCoordinate(m_viewportCenter)
        || !std::isfinite(dx)
        || !std::isfinite(dy)
        || !std::isfinite(viewportWidth)
        || !std::isfinite(viewportHeight)
        || viewportWidth <= 0.0
        || viewportHeight <= 0.0) {
        return;
    }

    const qreal worldPixels = kTileSizePixels * std::pow(2.0, m_zoomLevel);
    const qreal latitudeRadians = qDegreesToRadians(std::clamp(
        m_viewportCenter.latitude(),
        -kMaximumMercatorLatitude,
        kMaximumMercatorLatitude));
    const qreal currentX = (m_viewportCenter.longitude() + 180.0)
        / 360.0 * worldPixels;
    const qreal currentY = (1.0 - std::log(std::tan(latitudeRadians)
        + 1.0 / std::cos(latitudeRadians)) / M_PI) / 2.0 * worldPixels;
    const qreal nextX = currentX - dx;
    const qreal nextY = std::clamp(currentY - dy, 0.0, worldPixels);
    const qreal nextLongitude = wrappedLongitude(nextX / worldPixels * 360.0
                                                  - 180.0);
    const qreal mercatorY = M_PI * (1.0 - 2.0 * nextY / worldPixels);
    const qreal nextLatitude = std::clamp(
        qRadiansToDegrees(std::atan(std::sinh(mercatorY))),
        -kMaximumMercatorLatitude,
        kMaximumMercatorLatitude);

    enterExplore();
    setViewportCenter(QGeoCoordinate(nextLatitude, nextLongitude));
}

void MapViewModel::zoomByWheelDelta(qreal angleDeltaY)
{
    if (!std::isfinite(angleDeltaY) || qFuzzyIsNull(angleDeltaY)) {
        return;
    }

    enterExplore();
    setZoomLevel(m_zoomLevel + angleDeltaY / 120.0);
}

void MapViewModel::zoomByPinchScale(qreal scaleDelta)
{
    if (!std::isfinite(scaleDelta) || scaleDelta <= 0.0
        || qFuzzyCompare(scaleDelta, 1.0)) {
        return;
    }

    enterExplore();
    setZoomLevel(m_zoomLevel + std::log2(scaleDelta));
}

void MapViewModel::resumeFollow()
{
    m_followTimer.stop();
    m_remainingFollowMs = 0;
    if (!m_followEnabled) {
        m_followEnabled = true;
        emit followEnabledChanged();
    }
    if (isFiniteCoordinate(m_position)) {
        setViewportCenter(m_position);
    }
    setZoomLevel(kDefaultZoomLevel);
}

void MapViewModel::advanceFollowClock(qint64 elapsedMs)
{
    if (m_followEnabled || elapsedMs <= 0) {
        return;
    }

    if (elapsedMs >= m_remainingFollowMs) {
        resumeFollow();
        return;
    }
    m_remainingFollowMs -= elapsedMs;
}

void MapViewModel::handlePositionUpdated(const QGeoPositionInfo &info)
{
    const QGeoCoordinate coordinate = info.coordinate();
    const qreal direction = static_cast<qreal>(
        info.attribute(QGeoPositionInfo::Direction));
    if (!isFiniteCoordinate(coordinate)
        || !info.hasAttribute(QGeoPositionInfo::Direction)
        || !std::isfinite(direction)) {
        return;
    }

    const qreal bearing = normalizedBearing(direction);
    if (m_position != coordinate) {
        m_position = coordinate;
        emit positionChanged();
        if (m_followEnabled) {
            setViewportCenter(m_position);
        }
    }
    if (!qFuzzyCompare(m_bearingDegrees + 1.0, bearing + 1.0)) {
        m_bearingDegrees = bearing;
        emit bearingDegreesChanged();
    }
}

void MapViewModel::enterExplore()
{
    if (m_followEnabled) {
        m_followEnabled = false;
        emit followEnabledChanged();
    }
    m_remainingFollowMs = m_followTimeoutMs;
    m_followTimer.start();
}

void MapViewModel::setViewportCenter(const QGeoCoordinate &center)
{
    if (!isFiniteCoordinate(center)) {
        return;
    }
    const QGeoCoordinate boundedCenter(
        std::clamp(center.latitude(),
                   -kMaximumMercatorLatitude,
                   kMaximumMercatorLatitude),
        wrappedLongitude(center.longitude()),
        center.altitude());
    if (m_viewportCenter == boundedCenter) {
        return;
    }
    m_viewportCenter = boundedCenter;
    emit viewportCenterChanged();
}

void MapViewModel::setZoomLevel(qreal zoomLevel)
{
    if (!std::isfinite(zoomLevel)) {
        return;
    }

    const qreal boundedZoom = std::clamp(
        zoomLevel, kMinimumZoomLevel, kMaximumZoomLevel);
    if (qFuzzyCompare(m_zoomLevel + 1.0, boundedZoom + 1.0)) {
        return;
    }
    m_zoomLevel = boundedZoom;
    emit zoomLevelChanged();
}

qreal MapViewModel::normalizedBearing(qreal bearing)
{
    qreal normalized = std::fmod(bearing, 360.0);
    if (normalized < 0.0) {
        normalized += 360.0;
    }
    return normalized;
}

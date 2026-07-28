#pragma once

#include <QGeoCoordinate>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

class QGeoPositionInfo;
class QGeoPositionInfoSource;

class MapViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate position READ position NOTIFY positionChanged)
    Q_PROPERTY(qreal bearingDegrees READ bearingDegrees
                   NOTIFY bearingDegreesChanged)
    Q_PROPERTY(QGeoCoordinate viewportCenter READ viewportCenter
                   NOTIFY viewportCenterChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(bool followEnabled READ followEnabled NOTIFY followEnabledChanged)
    Q_PROPERTY(QString followLabel READ followLabel NOTIFY followEnabledChanged)

public:
    explicit MapViewModel(qint64 followTimeoutMs = 4000,
                          QObject *parent = nullptr);

    QGeoCoordinate position() const;
    qreal bearingDegrees() const;
    QGeoCoordinate viewportCenter() const;
    qreal zoomLevel() const;
    bool followEnabled() const;
    QString followLabel() const;

    void setPositionSource(QGeoPositionInfoSource *source);

    Q_INVOKABLE void panByPixels(qreal dx,
                                 qreal dy,
                                 qreal viewportWidth,
                                 qreal viewportHeight);
    Q_INVOKABLE void zoomByWheelDelta(qreal angleDeltaY,
                                      qreal pixelDeltaY);
    Q_INVOKABLE void zoomByPinchScale(qreal scaleDelta);
    Q_INVOKABLE void resumeFollow();
    Q_INVOKABLE void advanceFollowClock(qint64 elapsedMs);

signals:
    void positionChanged();
    void bearingDegreesChanged();
    void viewportCenterChanged();
    void zoomLevelChanged();
    void followEnabledChanged();

private:
    void handlePositionUpdated(const QGeoPositionInfo &info);
    void enterExplore();
    void setViewportCenter(const QGeoCoordinate &center);
    void setZoomLevel(qreal zoomLevel);
    static qreal normalizedBearing(qreal bearing);

    QPointer<QGeoPositionInfoSource> m_positionSource;
    QMetaObject::Connection m_positionConnection;
    QMetaObject::Connection m_destructionConnection;
    QGeoCoordinate m_position;
    qreal m_bearingDegrees = 0.0;
    QGeoCoordinate m_viewportCenter;
    qreal m_zoomLevel = 16.5;
    bool m_followEnabled = true;
    qint64 m_followTimeoutMs = 4000;
    qint64 m_remainingFollowMs = 0;
    QTimer m_followTimer;
};

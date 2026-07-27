#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class EncoderDriveViewModel final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal forwardSpeed READ forwardSpeed NOTIFY forwardSpeedChanged)
    Q_PROPERTY(qreal vehicleLateralOffset READ vehicleLateralOffset
                   NOTIFY vehicleLateralOffsetChanged)
    Q_PROPERTY(qreal vehicleYawDegrees READ vehicleYawDegrees
                   NOTIFY vehicleYawDegreesChanged)
    Q_PROPERTY(qreal frontWheelSteerDegrees READ frontWheelSteerDegrees
                   NOTIFY frontWheelSteerDegreesChanged)
    Q_PROPERTY(qreal roadCurvature READ roadCurvature NOTIFY roadCurvatureChanged)
    Q_PROPERTY(QString roadPath READ roadPath NOTIFY roadPathChanged)
    Q_PROPERTY(QString roadEdgePath READ roadEdgePath NOTIFY roadEdgePathChanged)
    Q_PROPERTY(TurnState turnState READ turnState NOTIFY turnStateChanged)

public:
    enum TurnState : int {
        Straight,
        GentleLeft,
        GentleRight,
        TurningLeft,
        TurningRight,
    };
    Q_ENUM(TurnState)

    explicit EncoderDriveViewModel(QObject *parent = nullptr);

    qreal forwardSpeed() const;
    qreal vehicleLateralOffset() const;
    qreal vehicleYawDegrees() const;
    qreal frontWheelSteerDegrees() const;
    qreal roadCurvature() const;
    QString roadPath() const;
    QString roadEdgePath() const;
    TurnState turnState() const;

public slots:
    void updateWheelMotion(double leftMotion,
                           double rightMotion,
                           qint64 elapsedMs);

signals:
    void forwardSpeedChanged();
    void vehicleLateralOffsetChanged();
    void vehicleYawDegreesChanged();
    void frontWheelSteerDegreesChanged();
    void roadCurvatureChanged();
    void roadPathChanged();
    void roadEdgePathChanged();
    void turnStateChanged();

private slots:
    void handleStaleTimeout();

private:
    void updateRoadPaths();
    static TurnState classifyTurn(double ratio, double signedDifference);
    static QString formatRoadPath(double horizonLeftX,
                                  double horizonY,
                                  double nearLeftX,
                                  double nearY,
                                  double nearRightX,
                                  double horizonRightX);
    static QString formatRoadEdgePath(double horizonLeftX,
                                      double horizonY,
                                      double nearLeftX,
                                      double nearY,
                                      double nearRightX,
                                      double horizonRightX);

    QTimer *m_staleTimer = nullptr;
    qreal m_forwardSpeed = 0.0;
    qreal m_vehicleLateralOffset = 0.0;
    qreal m_vehicleYawDegrees = 0.0;
    qreal m_frontWheelSteerDegrees = 0.0;
    qreal m_roadCurvature = 0.0;
    QString m_roadPath;
    QString m_roadEdgePath;
    TurnState m_turnState = Straight;
};

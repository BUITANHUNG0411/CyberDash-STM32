#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class ParkingAssistViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool reverseActive READ reverseActive NOTIFY reverseActiveChanged)
    Q_PROPERTY(bool sensorAvailable READ sensorAvailable NOTIFY sensorAvailableChanged)
    Q_PROPERTY(int rearDistanceCm READ rearDistanceCm NOTIFY rearDistanceChanged)
    Q_PROPERTY(ProximityLevel proximityLevel READ proximityLevel NOTIFY proximityLevelChanged)
    Q_PROPERTY(int proximitySegments READ proximitySegments NOTIFY proximitySegmentsChanged)
    Q_PROPERTY(double proximityProgress READ proximityProgress NOTIFY proximityProgressChanged)
    Q_PROPERTY(QString distanceText READ distanceText NOTIFY displayChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY displayChanged)

public:
    enum ProximityLevel {
        Unavailable,
        Clear,
        Caution,
        Stop
    };
    Q_ENUM(ProximityLevel)

    explicit ParkingAssistViewModel(qint64 staleIntervalMs = 1000, QObject *parent = nullptr);

    bool reverseActive() const;
    bool sensorAvailable() const;
    int rearDistanceCm() const;
    ProximityLevel proximityLevel() const;
    int proximitySegments() const;
    double proximityProgress() const;
    QString distanceText() const;
    QString statusText() const;

    void updateSensorSample(int distanceCm, bool reverseActive);
    void advanceStaleClock(qint64 elapsedMs);

signals:
    void reverseActiveChanged();
    void sensorAvailableChanged();
    void rearDistanceChanged();
    void proximityLevelChanged();
    void proximitySegmentsChanged();
    void proximityProgressChanged();
    void displayChanged();

private:
    void transitionToUnavailable();
    void emitDisplayChangedIfNeeded(const QString &previousDistanceText,
                                    const QString &previousStatusText);
    void emitPresentationChangedIfNeeded(int previousSegments, double previousProgress);

    bool m_reverseActive = false;
    bool m_sensorAvailable = false;
    int m_rearDistanceCm = 0;
    ProximityLevel m_proximityLevel = Unavailable;
    qint64 m_staleIntervalMs;
    qint64 m_staleRemainingMs = 0;
    QTimer m_staleTimer;
};

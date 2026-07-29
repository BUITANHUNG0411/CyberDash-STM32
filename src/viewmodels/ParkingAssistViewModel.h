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
    Q_PROPERTY(SensorHealth sensorHealth READ sensorHealth NOTIFY sensorHealthChanged)
    Q_PROPERTY(bool criticalProximity READ criticalProximity NOTIFY criticalProximityChanged)
    Q_PROPERTY(int proximitySegments READ proximitySegments NOTIFY proximitySegmentsChanged)
    Q_PROPERTY(double proximityProgress READ proximityProgress NOTIFY proximityProgressChanged)
    Q_PROPERTY(QString distanceText READ distanceText NOTIFY displayChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY displayChanged)
    Q_PROPERTY(QString healthText READ healthText NOTIFY displayChanged)

public:
    enum ProximityLevel {
        Unavailable,
        Clear,
        Caution,
        Stop
    };
    Q_ENUM(ProximityLevel)

    enum SensorHealth {
        SensorUnavailable,
        Live,
        Stale
    };
    Q_ENUM(SensorHealth)

    explicit ParkingAssistViewModel(qint64 staleIntervalMs = 1000, QObject *parent = nullptr);

    bool reverseActive() const;
    bool sensorAvailable() const;
    int rearDistanceCm() const;
    ProximityLevel proximityLevel() const;
    SensorHealth sensorHealth() const;
    bool criticalProximity() const;
    int proximitySegments() const;
    double proximityProgress() const;
    QString distanceText() const;
    QString statusText() const;
    QString healthText() const;

    Q_INVOKABLE QString formatDistance(double distanceCm) const;

    void updateSensorSample(int distanceCm, bool reverseActive);
    void advanceStaleClock(qint64 elapsedMs);

signals:
    void reverseActiveChanged();
    void sensorAvailableChanged();
    void rearDistanceChanged();
    void proximityLevelChanged();
    void sensorHealthChanged();
    void criticalProximityChanged();
    void proximitySegmentsChanged();
    void proximityProgressChanged();
    void displayChanged();

private:
    void transitionToUnavailable();
    void transitionToStale();
    ProximityLevel levelForDistance(int distanceCm) const;
    void emitDisplayChangedIfNeeded(const QString &previousDistanceText,
                                    const QString &previousStatusText,
                                    const QString &previousHealthText);
    void emitPresentationChangedIfNeeded(int previousSegments, double previousProgress);
    void emitCriticalProximityChangedIfNeeded(bool previousCriticalProximity);

    bool m_reverseActive = false;
    bool m_sensorAvailable = false;
    int m_rearDistanceCm = 0;
    ProximityLevel m_proximityLevel = Unavailable;
    SensorHealth m_sensorHealth = SensorUnavailable;
    qint64 m_staleIntervalMs;
    qint64 m_staleRemainingMs = 0;
    QTimer m_staleTimer;
};

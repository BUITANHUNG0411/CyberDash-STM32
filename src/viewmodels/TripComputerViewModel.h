#pragma once

#include <QObject>
#include <QString>

class TripComputerViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double odometerKm READ odometerKm NOTIFY tripChanged)
    Q_PROPERTY(double tripKm READ tripKm NOTIFY tripChanged)
    Q_PROPERTY(double avgSpeedKmh READ avgSpeedKmh NOTIFY tripChanged)
    Q_PROPERTY(QString tripDisplay READ tripDisplay NOTIFY tripChanged)
    Q_PROPERTY(QString odoDisplay READ odoDisplay NOTIFY tripChanged)

public:
    // maxDeltaMs clamps stale gaps (serial<->simulator source switch, UI stalls).
    explicit TripComputerViewModel(qint64 maxDeltaMs = 1000, QObject *parent = nullptr);

    double odometerKm() const;
    double tripKm() const;
    double avgSpeedKmh() const;
    QString tripDisplay() const; // "12.4 km"
    QString odoDisplay() const;  // "132 km"

    Q_INVOKABLE void resetTrip();

public slots:
    void updateSpeed(double speedKmh, qint64 elapsedMs);

signals:
    void tripChanged();

private:
    qint64 m_maxDeltaMs;
    double m_odometerKm = 0.0;
    double m_tripKm = 0.0;
    qint64 m_tripElapsedMs = 0;
};

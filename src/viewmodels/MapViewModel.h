#pragma once

#include <QObject>

class MapViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal routeProgress READ routeProgress NOTIFY routeProgressChanged)

public:
    // routeLengthKm: length of the map's route loop; injectable for tests.
    explicit MapViewModel(double routeLengthKm = 2.0, QObject *parent = nullptr);

    qreal routeProgress() const;

public slots:
    void updateDistance(double odometerKm);

signals:
    void routeProgressChanged();

private:
    double m_routeLengthKm;
    qreal m_routeProgress = 0.0;
};

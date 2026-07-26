#include "MapViewModel.h"

#include <cmath>

MapViewModel::MapViewModel(double routeLengthKm, QObject *parent)
    : QObject(parent), m_routeLengthKm(routeLengthKm) {}

qreal MapViewModel::routeProgress() const { return m_routeProgress; }

void MapViewModel::updateDistance(double odometerKm)
{
    const qreal progress = std::fmod(odometerKm / m_routeLengthKm, 1.0);
    if (qFuzzyCompare(m_routeProgress + 1.0, progress + 1.0)) {
        return;
    }
    m_routeProgress = progress;
    emit routeProgressChanged();
}

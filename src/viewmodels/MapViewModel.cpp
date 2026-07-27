#include "MapViewModel.h"

#include <cmath>

namespace {
constexpr double kDefaultRouteLengthKm = 2.0;
}

MapViewModel::MapViewModel(double routeLengthKm, QObject *parent)
    : QObject(parent),
      m_routeLengthKm(std::isfinite(routeLengthKm) && routeLengthKm > 0.0
                          ? routeLengthKm
                          : kDefaultRouteLengthKm)
{
}

qreal MapViewModel::routeProgress() const { return m_routeProgress; }

void MapViewModel::updateDistance(double odometerKm)
{
    qreal progress = 0.0;
    if (std::isfinite(odometerKm) && odometerKm >= 0.0) {
        const double routePosition = odometerKm / m_routeLengthKm;
        if (std::isfinite(routePosition)) {
            progress = std::fmod(routePosition, 1.0);
        }
    }

    if (qFuzzyCompare(m_routeProgress + 1.0, progress + 1.0)) {
        return;
    }
    m_routeProgress = progress;
    emit routeProgressChanged();
}

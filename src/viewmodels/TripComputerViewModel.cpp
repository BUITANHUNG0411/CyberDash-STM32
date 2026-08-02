#include "TripComputerViewModel.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kMsPerHour = 3600000.0;
constexpr int displayPrecision = 1;
constexpr qint64 minimumMaxDeltaMs = 1;
}

TripComputerViewModel::TripComputerViewModel(qint64 maxDeltaMs, QObject *parent)
    : QObject(parent), m_maxDeltaMs((std::max)(minimumMaxDeltaMs, maxDeltaMs)) {}

double TripComputerViewModel::odometerKm() const { return m_odometerKm; }
double TripComputerViewModel::tripKm() const { return m_tripKm; }

double TripComputerViewModel::avgSpeedKmh() const
{
    if (m_tripElapsedMs <= 0)
        return 0.0; // divide-by-zero guard: no time elapsed yet
    return m_tripKm / (static_cast<double>(m_tripElapsedMs) / kMsPerHour);
}

QString TripComputerViewModel::tripDisplay() const
{
    return QString::number(m_tripKm, 'f', displayPrecision) + QStringLiteral(" km");
}

QString TripComputerViewModel::odoDisplay() const
{
    return QString::number(qRound(m_odometerKm)) + QStringLiteral(" km");
}

QString TripComputerViewModel::avgSpeedDisplay() const
{
    return QString::number(avgSpeedKmh(), 'f', displayPrecision) + QStringLiteral(" km/h");
}

void TripComputerViewModel::updateSpeed(double speedKmh, qint64 elapsedMs)
{
    if (elapsedMs <= 0 || !std::isfinite(speedKmh))
        return;
    const qint64 dtMs = (std::min)(elapsedMs, m_maxDeltaMs);
    if (dtMs <= 0)
        return;

    const double previousOdometerKm = m_odometerKm;
    const double previousTripKm = m_tripKm;
    const double previousAverageSpeedKmh = avgSpeedKmh();
    const double distanceKm = (std::max)(0.0, speedKmh)
                              * (static_cast<double>(dtMs) / kMsPerHour);
    m_odometerKm += distanceKm;
    m_tripKm += distanceKm;
    m_tripElapsedMs += dtMs;

    if (m_odometerKm != previousOdometerKm
        || m_tripKm != previousTripKm
        || avgSpeedKmh() != previousAverageSpeedKmh) {
        emit tripChanged();
    }
}

void TripComputerViewModel::resetTrip()
{
    if (m_tripKm == 0.0 && m_tripElapsedMs == 0) {
        return;
    }

    m_tripKm = 0.0;
    m_tripElapsedMs = 0;
    emit tripChanged();
}

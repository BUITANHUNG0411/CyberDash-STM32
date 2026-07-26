#include "TripComputerViewModel.h"

namespace {
constexpr double kMsPerHour = 3600000.0;
}

TripComputerViewModel::TripComputerViewModel(qint64 maxDeltaMs, QObject *parent)
    : QObject(parent), m_maxDeltaMs(maxDeltaMs) {}

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
    return QString::number(m_tripKm, 'f', 1) + QStringLiteral(" km");
}

QString TripComputerViewModel::odoDisplay() const
{
    return QString::number(qRound(m_odometerKm)) + QStringLiteral(" km");
}

void TripComputerViewModel::updateSpeed(double speedKmh, qint64 elapsedMs)
{
    if (elapsedMs <= 0)
        return;
    const qint64 dtMs = qMin(elapsedMs, m_maxDeltaMs);
    const double distanceKm = qMax(0.0, speedKmh)
                              * (static_cast<double>(dtMs) / kMsPerHour);
    m_odometerKm += distanceKm;
    m_tripKm += distanceKm;
    m_tripElapsedMs += dtMs;
    emit tripChanged();
}

void TripComputerViewModel::resetTrip()
{
    m_tripKm = 0.0;
    m_tripElapsedMs = 0;
    emit tripChanged();
}

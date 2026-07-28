#include "ParkingAssistViewModel.h"

ParkingAssistViewModel::ParkingAssistViewModel(qint64 staleIntervalMs, QObject *parent)
    : QObject(parent), m_staleIntervalMs(staleIntervalMs)
{
    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(static_cast<int>(m_staleIntervalMs));
    connect(&m_staleTimer, &QTimer::timeout, this, &ParkingAssistViewModel::transitionToUnavailable);
}

bool ParkingAssistViewModel::reverseActive() const { return m_reverseActive; }

bool ParkingAssistViewModel::sensorAvailable() const { return m_sensorAvailable; }

int ParkingAssistViewModel::rearDistanceCm() const { return m_rearDistanceCm; }

ParkingAssistViewModel::ProximityLevel ParkingAssistViewModel::proximityLevel() const
{
    return m_proximityLevel;
}

QString ParkingAssistViewModel::distanceText() const
{
    if (!m_sensorAvailable) {
        return QStringLiteral("—");
    }

    return QString::number(m_rearDistanceCm) + QStringLiteral(" CM");
}

QString ParkingAssistViewModel::statusText() const
{
    switch (m_proximityLevel) {
    case Clear:
        return QStringLiteral("REAR CLEAR");
    case Caution:
        return QStringLiteral("CAUTION");
    case Stop:
        return QStringLiteral("STOP");
    case Unavailable:
        return QStringLiteral("SENSOR UNAVAILABLE");
    }

    return QStringLiteral("SENSOR UNAVAILABLE");
}

void ParkingAssistViewModel::updateSensorSample(int distanceCm, bool reverseActive)
{
    const QString previousDistanceText = distanceText();
    const QString previousStatusText = statusText();

    if (m_reverseActive != reverseActive) {
        m_reverseActive = reverseActive;
        emit reverseActiveChanged();
    }

    if (!reverseActive || distanceCm < 1 || distanceCm > 250) {
        transitionToUnavailable();
        return;
    }

    const ProximityLevel newLevel = distanceCm <= 30
        ? Stop
        : distanceCm <= 150 ? Caution : Clear;
    const bool availabilityChanged = !m_sensorAvailable;
    const bool distanceChanged = m_rearDistanceCm != distanceCm;
    const bool levelChanged = m_proximityLevel != newLevel;

    m_sensorAvailable = true;
    m_rearDistanceCm = distanceCm;
    m_proximityLevel = newLevel;
    m_staleRemainingMs = m_staleIntervalMs;
    m_staleTimer.start();

    if (availabilityChanged) {
        emit sensorAvailableChanged();
    }
    if (distanceChanged) {
        emit rearDistanceChanged();
    }
    if (levelChanged) {
        emit proximityLevelChanged();
    }
    emitDisplayChangedIfNeeded(previousDistanceText, previousStatusText);
}

void ParkingAssistViewModel::advanceStaleClock(qint64 elapsedMs)
{
    if (!m_sensorAvailable || elapsedMs <= 0) {
        return;
    }

    m_staleRemainingMs -= elapsedMs;
    if (m_staleRemainingMs <= 0) {
        transitionToUnavailable();
    }
}

void ParkingAssistViewModel::transitionToUnavailable()
{
    const QString previousDistanceText = distanceText();
    const QString previousStatusText = statusText();
    const bool availabilityChanged = m_sensorAvailable;
    const bool distanceChanged = m_rearDistanceCm != 0;
    const bool levelChanged = m_proximityLevel != Unavailable;

    m_staleTimer.stop();
    m_staleRemainingMs = 0;
    m_sensorAvailable = false;
    m_rearDistanceCm = 0;
    m_proximityLevel = Unavailable;

    if (availabilityChanged) {
        emit sensorAvailableChanged();
    }
    if (distanceChanged) {
        emit rearDistanceChanged();
    }
    if (levelChanged) {
        emit proximityLevelChanged();
    }
    emitDisplayChangedIfNeeded(previousDistanceText, previousStatusText);
}

void ParkingAssistViewModel::emitDisplayChangedIfNeeded(const QString &previousDistanceText,
                                                         const QString &previousStatusText)
{
    if (previousDistanceText != distanceText() || previousStatusText != statusText()) {
        emit displayChanged();
    }
}

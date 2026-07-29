#include "ParkingAssistViewModel.h"

#include <QtGlobal>

ParkingAssistViewModel::ParkingAssistViewModel(qint64 staleIntervalMs, QObject *parent)
    : QObject(parent), m_staleIntervalMs(staleIntervalMs)
{
    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(static_cast<int>(m_staleIntervalMs));
    connect(&m_staleTimer, &QTimer::timeout, this, &ParkingAssistViewModel::transitionToStale);
}

bool ParkingAssistViewModel::reverseActive() const { return m_reverseActive; }

bool ParkingAssistViewModel::sensorAvailable() const { return m_sensorAvailable; }

int ParkingAssistViewModel::rearDistanceCm() const { return m_rearDistanceCm; }

ParkingAssistViewModel::ProximityLevel ParkingAssistViewModel::proximityLevel() const
{
    return m_proximityLevel;
}

ParkingAssistViewModel::SensorHealth ParkingAssistViewModel::sensorHealth() const
{
    return m_sensorHealth;
}

bool ParkingAssistViewModel::criticalProximity() const
{
    return m_reverseActive && m_sensorAvailable && m_rearDistanceCm < 30;
}

int ParkingAssistViewModel::proximitySegments() const
{
    if (!m_sensorAvailable) {
        return 0;
    }

    return qBound(1, 1 + static_cast<int>(proximityProgress() * 7.0), 8);
}

double ParkingAssistViewModel::proximityProgress() const
{
    if (!m_sensorAvailable) {
        return 0.0;
    }

    constexpr double farDistanceCm = 250.0;
    constexpr double stopDistanceCm = 30.0;
    const double progress = (farDistanceCm - static_cast<double>(m_rearDistanceCm))
        / (farDistanceCm - stopDistanceCm);
    return qBound(0.0, progress, 1.0);
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
        return QStringLiteral("REAR CAUTION");
    case Stop:
        return QStringLiteral("STOP");
    case Unavailable:
        return m_sensorHealth == Stale
            ? QStringLiteral("SENSOR STALE")
            : QStringLiteral("SENSOR UNAVAILABLE");
    }

    return QStringLiteral("SENSOR UNAVAILABLE");
}

QString ParkingAssistViewModel::healthText() const
{
    switch (m_sensorHealth) {
    case Live:
        return QStringLiteral("ULTRASONIC LIVE");
    case Stale:
        return QStringLiteral("ULTRASONIC STALE");
    case SensorUnavailable:
        return QStringLiteral("ULTRASONIC UNAVAILABLE");
    }

    return QStringLiteral("ULTRASONIC UNAVAILABLE");
}

QString ParkingAssistViewModel::formatDistance(double distanceCm) const
{
    if (!qIsFinite(distanceCm) || distanceCm <= 0.0) {
        return QStringLiteral("—");
    }

    return QString::number(qRound64(distanceCm)) + QStringLiteral(" CM");
}

ParkingAssistViewModel::ProximityLevel ParkingAssistViewModel::levelForDistance(int distanceCm) const
{
    switch (m_proximityLevel) {
    case Clear:
        if (distanceCm <= 30) {
            return Stop;
        }
        return distanceCm <= 150 ? Caution : Clear;
    case Caution:
        if (distanceCm <= 30) {
            return Stop;
        }
        return distanceCm >= 155 ? Clear : Caution;
    case Stop:
        return distanceCm >= 35 ? Caution : Stop;
    case Unavailable:
        return distanceCm <= 30 ? Stop : distanceCm <= 150 ? Caution : Clear;
    }

    return Unavailable;
}

void ParkingAssistViewModel::updateSensorSample(int distanceCm, bool reverseActive)
{
    const QString previousDistanceText = distanceText();
    const QString previousStatusText = statusText();
    const QString previousHealthText = healthText();
    const int previousSegments = proximitySegments();
    const double previousProgress = proximityProgress();
    const bool previousCriticalProximity = criticalProximity();

    if (m_reverseActive != reverseActive) {
        m_reverseActive = reverseActive;
        emit reverseActiveChanged();
        emitCriticalProximityChangedIfNeeded(previousCriticalProximity);
    }

    if (!reverseActive || distanceCm < 1 || distanceCm > 250) {
        transitionToUnavailable();
        return;
    }

    const ProximityLevel newLevel = levelForDistance(distanceCm);
    const bool availabilityChanged = !m_sensorAvailable;
    const bool distanceChanged = m_rearDistanceCm != distanceCm;
    const bool levelChanged = m_proximityLevel != newLevel;
    const bool healthChanged = m_sensorHealth != Live;

    m_sensorAvailable = true;
    m_rearDistanceCm = distanceCm;
    m_proximityLevel = newLevel;
    m_sensorHealth = Live;
    m_staleRemainingMs = m_staleIntervalMs;
    m_staleTimer.start();

    if (availabilityChanged) {
        emit sensorAvailableChanged();
    }
    if (healthChanged) {
        emit sensorHealthChanged();
    }
    if (distanceChanged) {
        emit rearDistanceChanged();
    }
    if (levelChanged) {
        emit proximityLevelChanged();
    }
    emitDisplayChangedIfNeeded(previousDistanceText, previousStatusText, previousHealthText);
    emitPresentationChangedIfNeeded(previousSegments, previousProgress);
    emitCriticalProximityChangedIfNeeded(previousCriticalProximity);
}

void ParkingAssistViewModel::advanceStaleClock(qint64 elapsedMs)
{
    if (!m_sensorAvailable || elapsedMs <= 0) {
        return;
    }

    m_staleRemainingMs -= elapsedMs;
    if (m_staleRemainingMs <= 0) {
        transitionToStale();
    }
}

void ParkingAssistViewModel::transitionToUnavailable()
{
    const QString previousDistanceText = distanceText();
    const QString previousStatusText = statusText();
    const QString previousHealthText = healthText();
    const int previousSegments = proximitySegments();
    const double previousProgress = proximityProgress();
    const bool previousCriticalProximity = criticalProximity();
    const bool availabilityChanged = m_sensorAvailable;
    const bool distanceChanged = m_rearDistanceCm != 0;
    const bool levelChanged = m_proximityLevel != Unavailable;
    const bool healthChanged = m_sensorHealth != SensorUnavailable;

    m_staleTimer.stop();
    m_staleRemainingMs = 0;
    m_sensorAvailable = false;
    m_rearDistanceCm = 0;
    m_proximityLevel = Unavailable;
    m_sensorHealth = SensorUnavailable;

    if (availabilityChanged) {
        emit sensorAvailableChanged();
    }
    if (healthChanged) {
        emit sensorHealthChanged();
    }
    if (distanceChanged) {
        emit rearDistanceChanged();
    }
    if (levelChanged) {
        emit proximityLevelChanged();
    }
    emitDisplayChangedIfNeeded(previousDistanceText, previousStatusText, previousHealthText);
    emitPresentationChangedIfNeeded(previousSegments, previousProgress);
    emitCriticalProximityChangedIfNeeded(previousCriticalProximity);
}

void ParkingAssistViewModel::transitionToStale()
{
    const QString previousDistanceText = distanceText();
    const QString previousStatusText = statusText();
    const QString previousHealthText = healthText();
    const int previousSegments = proximitySegments();
    const double previousProgress = proximityProgress();
    const bool previousCriticalProximity = criticalProximity();
    const bool availabilityChanged = m_sensorAvailable;
    const bool distanceChanged = m_rearDistanceCm != 0;
    const bool levelChanged = m_proximityLevel != Unavailable;
    const bool healthChanged = m_sensorHealth != Stale;

    m_staleTimer.stop();
    m_staleRemainingMs = 0;
    m_sensorAvailable = false;
    m_rearDistanceCm = 0;
    m_proximityLevel = Unavailable;
    m_sensorHealth = Stale;

    if (availabilityChanged) {
        emit sensorAvailableChanged();
    }
    if (healthChanged) {
        emit sensorHealthChanged();
    }
    if (distanceChanged) {
        emit rearDistanceChanged();
    }
    if (levelChanged) {
        emit proximityLevelChanged();
    }
    emitDisplayChangedIfNeeded(previousDistanceText, previousStatusText, previousHealthText);
    emitPresentationChangedIfNeeded(previousSegments, previousProgress);
    emitCriticalProximityChangedIfNeeded(previousCriticalProximity);
}

void ParkingAssistViewModel::emitDisplayChangedIfNeeded(const QString &previousDistanceText,
                                                         const QString &previousStatusText,
                                                         const QString &previousHealthText)
{
    if (previousDistanceText != distanceText()
        || previousStatusText != statusText()
        || previousHealthText != healthText()) {
        emit displayChanged();
    }
}

void ParkingAssistViewModel::emitPresentationChangedIfNeeded(int previousSegments,
                                                              double previousProgress)
{
    if (previousSegments != proximitySegments()) {
        emit proximitySegmentsChanged();
    }
    if (previousProgress != proximityProgress()) {
        emit proximityProgressChanged();
    }
}

void ParkingAssistViewModel::emitCriticalProximityChangedIfNeeded(bool previousCriticalProximity)
{
    if (previousCriticalProximity != criticalProximity()) {
        emit criticalProximityChanged();
    }
}

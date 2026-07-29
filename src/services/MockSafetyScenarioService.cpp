#include "MockSafetyScenarioService.h"

#include <algorithm>
#include <QTimer>

namespace {
constexpr qint64 kAdvisoryStartMs = 6000;
constexpr qint64 kCriticalStartMs = 20000;
constexpr qint64 kAcknowledgementStartMs = 34000;
constexpr qint64 kRecoveryStartMs = 48000;
constexpr qint64 kCompleteMs = 72000;
constexpr int kMaximumRiskSegments = 8;

qreal normalizedProgress(qint64 value, qint64 start, qint64 end)
{
    return static_cast<qreal>(value - start) / static_cast<qreal>(end - start);
}

int riskSegmentsFor(qreal riskProgress)
{
    const int segments = static_cast<int>(riskProgress * static_cast<qreal>(kMaximumRiskSegments) + 0.5);
    return std::clamp(segments, 0, kMaximumRiskSegments);
}
} // namespace

MockSafetyScenarioService::MockSafetyScenarioService(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(static_cast<int>(updateIntervalMs()));
    connect(m_timer, &QTimer::timeout, this, [this] { advance(updateIntervalMs()); });
}

void MockSafetyScenarioService::start()
{
    m_elapsedMs = 0;
    if (!m_running) {
        m_running = true;
        emit runningChanged();
    }

    updateFrame();
    m_timer->start();
}

void MockSafetyScenarioService::stop()
{
    m_timer->stop();
    m_elapsedMs = 0;
    if (m_running) {
        m_running = false;
        emit runningChanged();
    }

    updateFrame();
}

void MockSafetyScenarioService::replay()
{
    start();
}

bool MockSafetyScenarioService::acknowledge()
{
    if (!m_acknowledgementAvailable) {
        return false;
    }

    m_elapsedMs = kRecoveryStartMs;
    updateFrame();
    return true;
}

void MockSafetyScenarioService::advance(qint64 elapsedMs)
{
    if (!m_running || elapsedMs <= 0) {
        return;
    }

    if (m_elapsedMs >= kCompleteMs || elapsedMs >= kCompleteMs - m_elapsedMs) {
        m_elapsedMs = kCompleteMs;
    } else {
        m_elapsedMs += elapsedMs;
    }
    updateFrame();

    if (m_elapsedMs == kCompleteMs) {
        m_timer->stop();
        if (m_running) {
            m_running = false;
            emit runningChanged();
        }
    }
}

bool MockSafetyScenarioService::isRunning() const
{
    return m_running;
}

MockSafetyScenarioService::Phase MockSafetyScenarioService::phase() const
{
    return m_phase;
}

MockSafetyScenarioService::Severity MockSafetyScenarioService::severity() const
{
    return m_severity;
}

qreal MockSafetyScenarioService::riskProgress() const
{
    return m_riskProgress;
}

int MockSafetyScenarioService::riskSegments() const
{
    return m_riskSegments;
}

qreal MockSafetyScenarioService::threatPosition() const
{
    return m_threatPosition;
}

bool MockSafetyScenarioService::pulseActive() const
{
    return m_pulseActive;
}

bool MockSafetyScenarioService::acknowledgementAvailable() const
{
    return m_acknowledgementAvailable;
}

void MockSafetyScenarioService::updateFrame()
{
    Phase phase = Phase::Idle;
    Severity severity = Severity::None;
    qreal riskProgress = 0.0;
    qreal threatPosition = 0.0;
    bool pulseActive = false;
    bool acknowledgementAvailable = false;

    if (m_running) {
        threatPosition = normalizedProgress(m_elapsedMs, 0, kCompleteMs);
        if (m_elapsedMs < kAdvisoryStartMs) {
            phase = Phase::Normal;
        } else if (m_elapsedMs < kCriticalStartMs) {
            phase = Phase::Advisory;
            severity = Severity::Advisory;
            riskProgress = 0.20 + (0.50 * normalizedProgress(m_elapsedMs, kAdvisoryStartMs, kCriticalStartMs));
        } else if (m_elapsedMs < kRecoveryStartMs) {
            phase = Phase::Critical;
            severity = Severity::Critical;
            riskProgress = 1.0;
            pulseActive = true;
            acknowledgementAvailable = m_elapsedMs >= kAcknowledgementStartMs;
        } else if (m_elapsedMs < kCompleteMs) {
            phase = Phase::Recovery;
            severity = Severity::Advisory;
            riskProgress = 1.0 - normalizedProgress(m_elapsedMs, kRecoveryStartMs, kCompleteMs);
        } else {
            phase = Phase::Complete;
        }
    }

    const int riskSegments = riskSegmentsFor(riskProgress);
    const bool changed = m_phase != phase
        || m_severity != severity
        || m_riskProgress != riskProgress
        || m_riskSegments != riskSegments
        || m_threatPosition != threatPosition
        || m_pulseActive != pulseActive
        || m_acknowledgementAvailable != acknowledgementAvailable;
    if (!changed) {
        return;
    }

    m_phase = phase;
    m_severity = severity;
    m_riskProgress = riskProgress;
    m_riskSegments = riskSegments;
    m_threatPosition = threatPosition;
    m_pulseActive = pulseActive;
    m_acknowledgementAvailable = acknowledgementAvailable;
    emit frameChanged();
}

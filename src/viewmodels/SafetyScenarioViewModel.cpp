#include "SafetyScenarioViewModel.h"

#include "services/MockSafetyScenarioService.h"

namespace {
QString titleFor(MockSafetyScenarioService::Phase phase)
{
    switch (phase) {
    case MockSafetyScenarioService::Phase::Advisory:
        return QStringLiteral("FORWARD HAZARD");
    case MockSafetyScenarioService::Phase::Critical:
        return QStringLiteral("BRAKE ADVISORY");
    case MockSafetyScenarioService::Phase::Recovery:
        return QStringLiteral("RISK CLEARING");
    case MockSafetyScenarioService::Phase::Idle:
    case MockSafetyScenarioService::Phase::Normal:
    case MockSafetyScenarioService::Phase::Complete:
        return QStringLiteral("SAFETY LAB");
    }

    return QStringLiteral("SAFETY LAB");
}

QString instructionFor(MockSafetyScenarioService::Phase phase)
{
    switch (phase) {
    case MockSafetyScenarioService::Phase::Advisory:
        return QStringLiteral("CAUTION: CLOSING DISTANCE");
    case MockSafetyScenarioService::Phase::Critical:
        return QStringLiteral("ACKNOWLEDGE TO BEGIN RECOVERY");
    case MockSafetyScenarioService::Phase::Recovery:
        return QStringLiteral("HAZARD WINDOW REDUCING");
    case MockSafetyScenarioService::Phase::Idle:
    case MockSafetyScenarioService::Phase::Normal:
    case MockSafetyScenarioService::Phase::Complete:
        return QStringLiteral("FORWARD HAZARD SIMULATION");
    }

    return QStringLiteral("FORWARD HAZARD SIMULATION");
}
} // namespace

SafetyScenarioViewModel::SafetyScenarioViewModel(MockSafetyScenarioService *service, QObject *parent)
    : QObject(parent)
    , m_service(service)
{
    if (!m_service) {
        return;
    }

    connect(m_service, &MockSafetyScenarioService::frameChanged,
            this, &SafetyScenarioViewModel::synchronizeFromService);
    connect(m_service, &MockSafetyScenarioService::runningChanged,
            this, &SafetyScenarioViewModel::synchronizeFromService);
    connect(m_service, &QObject::destroyed, this, [this](QObject *) {
        m_service = nullptr;
        synchronizeFromService();
    });
    synchronizeFromService();
}

bool SafetyScenarioViewModel::isActive() const { return m_active; }
bool SafetyScenarioViewModel::presentationVisible() const { return m_presentationVisible; }
bool SafetyScenarioViewModel::canStart() const { return m_canStart; }
QString SafetyScenarioViewModel::title() const { return m_title; }
QString SafetyScenarioViewModel::instructionText() const { return m_instructionText; }
QString SafetyScenarioViewModel::disclaimerText() const
{
    return QStringLiteral("DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL");
}
qreal SafetyScenarioViewModel::riskProgress() const { return m_riskProgress; }
int SafetyScenarioViewModel::riskSegments() const { return m_riskSegments; }
qreal SafetyScenarioViewModel::threatPosition() const { return m_threatPosition; }
bool SafetyScenarioViewModel::advisoryActive() const { return m_advisoryActive; }
bool SafetyScenarioViewModel::criticalActive() const { return m_criticalActive; }
bool SafetyScenarioViewModel::pulseActive() const { return m_pulseActive; }
bool SafetyScenarioViewModel::acknowledgementAvailable() const { return m_acknowledgementAvailable; }

void SafetyScenarioViewModel::setPresentationAllowed(bool allowed)
{
    if (m_presentationAllowed == allowed) {
        return;
    }

    m_presentationAllowed = allowed;
    if (!m_presentationAllowed && m_service && m_service->isRunning()) {
        m_service->stop();
    }
    synchronizeFromService();
}

void SafetyScenarioViewModel::startDemo()
{
    if (canStart() && m_service) {
        m_service->start();
    }
}

void SafetyScenarioViewModel::replay()
{
    if (m_presentationAllowed && m_service) {
        m_service->replay();
    }
}

void SafetyScenarioViewModel::acknowledge()
{
    if (m_presentationAllowed && m_service) {
        m_service->acknowledge();
    }
}

void SafetyScenarioViewModel::stopDemo()
{
    if (m_service) {
        m_service->stop();
    }
}

void SafetyScenarioViewModel::synchronizeFromService()
{
    const bool active = m_service && m_service->isRunning();
    const bool presentationVisible = active && m_presentationAllowed;
    const bool canStart = m_presentationAllowed && !active;
    const auto phase = m_service ? m_service->phase() : MockSafetyScenarioService::Phase::Idle;
    const auto severity = m_service ? m_service->severity() : MockSafetyScenarioService::Severity::None;
    const QString title = titleFor(phase);
    const QString instructionText = instructionFor(phase);
    const qreal riskProgress = m_service ? m_service->riskProgress() : 0.0;
    const int riskSegments = m_service ? m_service->riskSegments() : 0;
    const qreal threatPosition = m_service ? m_service->threatPosition() : 0.0;
    const bool advisoryActive = severity == MockSafetyScenarioService::Severity::Advisory;
    const bool criticalActive = severity == MockSafetyScenarioService::Severity::Critical;
    const bool pulseActive = m_service && m_service->pulseActive();
    const bool acknowledgementAvailable = m_service && m_service->acknowledgementAvailable();

    const bool activeChanged = m_active != active;
    const bool presentationVisibleChanged = m_presentationVisible != presentationVisible;
    const bool canStartChanged = m_canStart != canStart;
    const bool displayChanged = m_title != title || m_instructionText != instructionText;
    const bool riskProgressChanged = m_riskProgress != riskProgress;
    const bool riskSegmentsChanged = m_riskSegments != riskSegments;
    const bool threatPositionChanged = m_threatPosition != threatPosition;
    const bool advisoryActiveChanged = m_advisoryActive != advisoryActive;
    const bool criticalActiveChanged = m_criticalActive != criticalActive;
    const bool pulseActiveChanged = m_pulseActive != pulseActive;
    const bool acknowledgementAvailableChanged = m_acknowledgementAvailable != acknowledgementAvailable;

    m_active = active;
    m_presentationVisible = presentationVisible;
    m_canStart = canStart;
    m_title = title;
    m_instructionText = instructionText;
    m_riskProgress = riskProgress;
    m_riskSegments = riskSegments;
    m_threatPosition = threatPosition;
    m_advisoryActive = advisoryActive;
    m_criticalActive = criticalActive;
    m_pulseActive = pulseActive;
    m_acknowledgementAvailable = acknowledgementAvailable;

    if (activeChanged) {
        emit this->activeChanged();
    }
    if (presentationVisibleChanged) {
        emit this->presentationVisibleChanged();
    }
    if (canStartChanged) {
        emit this->canStartChanged();
    }
    if (displayChanged) {
        emit this->displayChanged();
    }
    if (riskProgressChanged) {
        emit this->riskProgressChanged();
    }
    if (riskSegmentsChanged) {
        emit this->riskSegmentsChanged();
    }
    if (threatPositionChanged) {
        emit this->threatPositionChanged();
    }
    if (advisoryActiveChanged) {
        emit this->advisoryActiveChanged();
    }
    if (criticalActiveChanged) {
        emit this->criticalActiveChanged();
    }
    if (pulseActiveChanged) {
        emit this->pulseActiveChanged();
    }
    if (acknowledgementAvailableChanged) {
        emit this->acknowledgementAvailableChanged();
    }
}

#include "MockWheelTelemetryService.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr qint64 kDefaultStageDurationMs = 4000;
constexpr qint64 kDefaultTransitionDurationMs = 1000;
constexpr std::chrono::milliseconds kDefaultTimerInterval{33};
constexpr std::chrono::milliseconds kMaximumTimerInterval{100};
constexpr qint64 kMaximumAdvanceMs = 100;
constexpr std::array<WheelMotionTarget, 7> kDefaultTargets{{
    {1.00, 1.00},
    {0.90, 1.00},
    {1.00, 1.00},
    {1.00, 0.90},
    {0.55, 1.00},
    {1.00, 1.00},
    {1.00, 0.55}
}};
}

MockWheelTelemetryService::MockWheelTelemetryService(
    const MockWheelTelemetryConfig &config,
    QObject *parent)
    : QObject(parent),
      m_config(config)
{
    if (m_config.stageDurationMs <= 0) {
        m_config.stageDurationMs = kDefaultStageDurationMs;
    }
    if (m_config.transitionDurationMs <= 0) {
        m_config.transitionDurationMs = kDefaultTransitionDurationMs;
    }
    m_config.transitionDurationMs =
        (std::min)(m_config.transitionDurationMs, m_config.stageDurationMs);
    if (m_config.timerInterval <= std::chrono::milliseconds::zero()) {
        m_config.timerInterval = kDefaultTimerInterval;
    }
    m_config.timerInterval =
        (std::min)(m_config.timerInterval, kMaximumTimerInterval);
    const std::size_t stageCount = m_config.targets.size();
    for (std::size_t index = 0; index < stageCount; ++index) {
        WheelMotionTarget &target = m_config.targets[index];
        const WheelMotionTarget fallback =
            kDefaultTargets[index % stageCount];
        target.left = std::isfinite(target.left)
            ? std::clamp(target.left, 0.0, 1.0)
            : fallback.left;
        target.right = std::isfinite(target.right)
            ? std::clamp(target.right, 0.0, 1.0)
            : fallback.right;
    }

    m_timer.setInterval(m_config.timerInterval);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        advance(static_cast<qint64>(
            m_config.timerInterval / std::chrono::milliseconds{1}));
    });
}

void MockWheelTelemetryService::start()
{
    m_timer.start();
}

void MockWheelTelemetryService::stop()
{
    m_timer.stop();
}

void MockWheelTelemetryService::advance(qint64 elapsedMs)
{
    if (elapsedMs <= 0) {
        return;
    }

    const qint64 acceptedElapsedMs =
        (std::min)(elapsedMs, kMaximumAdvanceMs);
    advanceStageClock(acceptedElapsedMs);
    const WheelMotionTarget target = targetForStage(m_stage);
    const WheelMotionTarget previous =
        m_stage == 0 && !m_completedCycle
            ? target
            : previousTargetForStage(m_stage);
    const double ratio = std::clamp(
        static_cast<double>(m_stageElapsedMs)
            / static_cast<double>(m_config.transitionDurationMs),
        0.0,
        1.0);

    emit wheelTelemetryUpdated(
        previous.left + (target.left - previous.left) * ratio,
        previous.right + (target.right - previous.right) * ratio,
        acceptedElapsedMs);
}

WheelMotionTarget
MockWheelTelemetryService::targetForStage(int stage) const
{
    const int stageCount = static_cast<int>(m_config.targets.size());
    const int normalizedStage = stage % stageCount;
    return m_config.targets.at(static_cast<std::size_t>(normalizedStage));
}

WheelMotionTarget
MockWheelTelemetryService::previousTargetForStage(int stage) const
{
    const int stageCount = static_cast<int>(m_config.targets.size());
    const int previousStage = stage == 0 ? stageCount - 1 : stage - 1;
    return targetForStage(previousStage);
}

void MockWheelTelemetryService::advanceStageClock(qint64 elapsedMs)
{
    const int stageCount = static_cast<int>(m_config.targets.size());
    m_stageElapsedMs += elapsedMs;
    while (m_stageElapsedMs >= m_config.stageDurationMs) {
        m_stageElapsedMs -= m_config.stageDurationMs;
        m_stage = (m_stage + 1) % stageCount;
        if (m_stage == 0) {
            m_completedCycle = true;
        }
    }
}

#include "MockWheelTelemetryService.h"

#include <algorithm>
#include <array>
#include <utility>

namespace {
constexpr qint64 kDefaultStageDurationMs = 4000;
constexpr qint64 kDefaultTransitionDurationMs = 1000;
constexpr int kDefaultTimerIntervalMs = 33;
constexpr int kStageCount = 4;
constexpr std::array<std::pair<double, double>, kStageCount> kTargets{{
    {1.0, 1.0},
    {0.65, 1.0},
    {1.0, 1.0},
    {1.0, 0.65}
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
        std::min(m_config.transitionDurationMs, m_config.stageDurationMs);
    if (m_config.timerIntervalMs <= 0) {
        m_config.timerIntervalMs = kDefaultTimerIntervalMs;
    }

    m_timer.setInterval(m_config.timerIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        advance(m_config.timerIntervalMs);
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

    advanceStageClock(elapsedMs);
    const WheelTarget target = targetForStage(m_stage);
    const WheelTarget previous =
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
        elapsedMs);
}

MockWheelTelemetryService::WheelTarget
MockWheelTelemetryService::targetForStage(int stage) const
{
    const auto &target = kTargets.at(static_cast<std::size_t>(stage));
    return {target.first, target.second};
}

MockWheelTelemetryService::WheelTarget
MockWheelTelemetryService::previousTargetForStage(int stage) const
{
    const int previousStage = stage == 0 ? kStageCount - 1 : stage - 1;
    return targetForStage(previousStage);
}

void MockWheelTelemetryService::advanceStageClock(qint64 elapsedMs)
{
    m_stageElapsedMs += elapsedMs;
    while (m_stageElapsedMs >= m_config.stageDurationMs) {
        m_stageElapsedMs -= m_config.stageDurationMs;
        m_stage = (m_stage + 1) % kStageCount;
        if (m_stage == 0) {
            m_completedCycle = true;
        }
    }
}

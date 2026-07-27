#pragma once

#include <QObject>
#include <QTimer>

#include <array>
#include <chrono>

struct WheelMotionTarget
{
    double left = 1.0;
    double right = 1.0;
};

struct MockWheelTelemetryConfig
{
    qint64 stageDurationMs = 4000;
    qint64 transitionDurationMs = 1000;
    std::chrono::milliseconds timerInterval = std::chrono::milliseconds{33};
    std::array<WheelMotionTarget, 4> targets = {{
        {1.0, 1.0},
        {0.65, 1.0},
        {1.0, 1.0},
        {1.0, 0.65}
    }};
};

class MockWheelTelemetryService final : public QObject
{
    Q_OBJECT

public:
    explicit MockWheelTelemetryService(
        const MockWheelTelemetryConfig &config = {},
        QObject *parent = nullptr);

    void start();
    void stop();
    void advance(qint64 elapsedMs);

signals:
    void wheelTelemetryUpdated(double leftWheelSpeed,
                               double rightWheelSpeed,
                               qint64 elapsedMs);

private:
    WheelMotionTarget targetForStage(int stage) const;
    WheelMotionTarget previousTargetForStage(int stage) const;
    void advanceStageClock(qint64 elapsedMs);

    MockWheelTelemetryConfig m_config;
    QTimer m_timer;
    qint64 m_stageElapsedMs = 0;
    int m_stage = 0;
    bool m_completedCycle = false;
};

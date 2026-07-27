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
    std::array<WheelMotionTarget, 7> targets = {{
        {1.00, 1.00}, // straight
        {0.90, 1.00}, // gentle left
        {1.00, 1.00}, // straight
        {1.00, 0.90}, // gentle right
        {0.55, 1.00}, // strong left
        {1.00, 1.00}, // straight
        {1.00, 0.55}  // strong right
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

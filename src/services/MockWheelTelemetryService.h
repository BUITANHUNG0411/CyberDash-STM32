#pragma once

#include <QObject>
#include <QTimer>

struct MockWheelTelemetryConfig
{
    qint64 stageDurationMs = 4000;
    qint64 transitionDurationMs = 1000;
    int timerIntervalMs = 33;
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
    struct WheelTarget {
        double left;
        double right;
    };

    WheelTarget targetForStage(int stage) const;
    WheelTarget previousTargetForStage(int stage) const;
    void advanceStageClock(qint64 elapsedMs);

    MockWheelTelemetryConfig m_config;
    QTimer m_timer;
    qint64 m_stageElapsedMs = 0;
    int m_stage = 0;
    bool m_completedCycle = false;
};

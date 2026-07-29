#pragma once

#include <QObject>

class QTimer;

class MockSafetyScenarioService final : public QObject
{
    Q_OBJECT

public:
    enum class Phase { Idle, Normal, Advisory, Critical, Recovery, Complete };
    Q_ENUM(Phase)

    enum class Severity { None, Advisory, Critical };
    Q_ENUM(Severity)

    explicit MockSafetyScenarioService(QObject *parent = nullptr);

    static constexpr qint64 updateIntervalMs() { return 100; }

    void start();
    void stop();
    void replay();
    bool acknowledge();
    void advance(qint64 elapsedMs);

    bool isRunning() const;
    Phase phase() const;
    Severity severity() const;
    qreal riskProgress() const;
    int riskSegments() const;
    qreal threatPosition() const;
    bool pulseActive() const;
    bool acknowledgementAvailable() const;

signals:
    void frameChanged();
    void runningChanged();

private:
    void updateFrame();

    QTimer *m_timer = nullptr;
    qint64 m_elapsedMs = 0;
    bool m_running = false;
    Phase m_phase = Phase::Idle;
    Severity m_severity = Severity::None;
    qreal m_riskProgress = 0.0;
    int m_riskSegments = 0;
    qreal m_threatPosition = 0.0;
    bool m_pulseActive = false;
    bool m_acknowledgementAvailable = false;
};

#pragma once

#include <QObject>
#include <QTimer>

class MockParkingSensorService final : public QObject
{
    Q_OBJECT

public:
    explicit MockParkingSensorService(QObject *parent = nullptr);

    static constexpr qint64 updateIntervalMs() { return 700; }

    void start();
    void stop();
    void advance(qint64 elapsedMs);

signals:
    void parkingSampleUpdated(int distanceCm, bool reverseActive);

private:
    void emitCurrentSample();
    void advanceOneSample();

    QTimer m_timer;
    int m_sampleIndex = 0;
    qint64 m_pendingElapsedMs = 0;
    bool m_running = false;
};

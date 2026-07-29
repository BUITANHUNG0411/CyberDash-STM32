#include "MockParkingSensorService.h"

#include <array>

namespace {
constexpr std::array<std::pair<int, bool>, 5> kSamples{{
    {0, false},
    {250, true},
    {150, true},
    {0, true},
    {0, false},
}};
}

MockParkingSensorService::MockParkingSensorService(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(static_cast<int>(updateIntervalMs()));
    connect(&m_timer, &QTimer::timeout, this, &MockParkingSensorService::advanceOneSample);
}

void MockParkingSensorService::start()
{
    m_sampleIndex = 0;
    m_pendingElapsedMs = 0;
    m_running = true;
    emitCurrentSample();
    m_timer.start();
}

void MockParkingSensorService::stop()
{
    m_timer.stop();
    m_pendingElapsedMs = 0;
    m_running = false;
}

void MockParkingSensorService::advance(qint64 elapsedMs)
{
    if (!m_running || elapsedMs <= 0) {
        return;
    }

    m_pendingElapsedMs += elapsedMs;
    while (m_pendingElapsedMs >= updateIntervalMs()) {
        m_pendingElapsedMs -= updateIntervalMs();
        advanceOneSample();
    }
}

void MockParkingSensorService::emitCurrentSample()
{
    const auto &sample = kSamples[static_cast<std::size_t>(m_sampleIndex)];
    emit parkingSampleUpdated(sample.first, sample.second);
}

void MockParkingSensorService::advanceOneSample()
{
    if (!m_running) {
        return;
    }

    m_sampleIndex = (m_sampleIndex + 1) % static_cast<int>(kSamples.size());
    emitCurrentSample();
}

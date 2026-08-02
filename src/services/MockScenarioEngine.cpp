#include "MockScenarioEngine.h"
#include <cmath>
#include <algorithm>

MockScenarioEngine::MockScenarioEngine() 
    : m_elapsedTimeMs(0)
{
}

MockTelemetry MockScenarioEngine::getCurrentTelemetry() const {
    return m_telemetry;
}

void MockScenarioEngine::tick(double deltaTimeMs) {
    m_elapsedTimeMs += deltaTimeMs;

    updateDragRace();
}

void MockScenarioEngine::updateDragRace() {
    // Accelerate from 0 to 160 over 8s, brake over 2s
    double t = std::fmod(m_elapsedTimeMs / 1000.0, 10.0);
    
    m_telemetry.isWarning = false;
    m_telemetry.temperature = 25 + (int)(t * 2);
    m_telemetry.battery = 100;
    m_telemetry.range = 400;
    
    if (t < 8.0) { // Accelerate
        m_telemetry.speed = (std::min)(160.0f, static_cast<float>(t * 20.0));
        // Gear calculation (every 32 km/h)
        float gearSpeed = std::fmod(m_telemetry.speed, 32.0f);
        m_telemetry.rpm = 1000.0f + (gearSpeed / 32.0f) * 5000.0f;
        m_telemetry.gear = 1 + (int)(m_telemetry.speed / 32.0f);
    } else { // Brake
        const double brakeTime = t - 8.0;
        m_telemetry.speed = (std::max)(0.0f, static_cast<float>(160.0 - (brakeTime * 80.0)));
        m_telemetry.rpm = 1000.0f + (m_telemetry.speed / 160.0f) * 2000.0f;
        m_telemetry.gear = (std::max)(1, static_cast<int>(m_telemetry.speed / 32.0f));
    }
    
    if (m_telemetry.gear > 5) m_telemetry.gear = 5;
}

#pragma once

#include <QString>

struct MockTelemetry {
    float speed = 0.0f;
    float rpm = 0.0f;
    int gear = 1;
    bool isWarning = false;
    int battery = 100;
    int range = 400;
    int temperature = 25;
};

class MockScenarioEngine {
public:
    MockScenarioEngine();
    void tick(double deltaTimeMs);

    MockTelemetry getCurrentTelemetry() const;

private:
    MockTelemetry m_telemetry;
    
    double m_elapsedTimeMs;
    
    void updateDragRace();
};

#pragma once

#include <QString>

#include "SerialTelemetryParser.h"

struct DashboardTelemetry
{
    double speed;
    int rpm;
    QString gear;
    bool warning;
    int battery;
    int range;
    int temperature;
};

class TelemetryMapper
{
public:
    static DashboardTelemetry fromSerial(const RawSerialTelemetry &raw);
};

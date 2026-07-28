#include "TelemetryMapper.h"

DashboardTelemetry TelemetryMapper::fromSerial(const RawSerialTelemetry &raw)
{
    const double speed = static_cast<double>(raw.rpm) * 0.03;
    QString gear = QStringLiteral("N");
    if (speed > 80.0) {
        gear = QStringLiteral("5");
    } else if (speed > 60.0) {
        gear = QStringLiteral("4");
    } else if (speed > 40.0) {
        gear = QStringLiteral("3");
    } else if (speed > 20.0) {
        gear = QStringLiteral("2");
    } else if (speed > 0.0) {
        gear = QStringLiteral("1");
    }

    return {speed, raw.rpm, gear,
            raw.errorCode != 0 || raw.batteryVoltage < 10.5,
            100, 325, 57};
}

#pragma once

#include <QByteArray>
#include <QList>

#include <optional>

struct RawSerialTelemetry
{
    int rpm;
    double batteryVoltage;
    int errorCode;
};

class SerialTelemetryParser
{
public:
    QList<RawSerialTelemetry> append(const QByteArray &bytes);
    void clear();

private:
    std::optional<RawSerialTelemetry> parseLine(const QString &line) const;

    QByteArray m_buffer;
};

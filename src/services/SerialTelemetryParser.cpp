#include "SerialTelemetryParser.h"

#include <QStringList>

QList<RawSerialTelemetry> SerialTelemetryParser::append(const QByteArray &bytes)
{
    m_buffer.append(bytes);
    if (m_buffer.size() > 4096) {
        m_buffer.clear();
        return {};
    }

    QList<RawSerialTelemetry> result;
    while (m_buffer.contains('\n')) {
        const qsizetype newline = m_buffer.indexOf('\n');
        const QString line = QString::fromUtf8(m_buffer.first(newline)).trimmed();
        m_buffer.remove(0, newline + 1);
        const auto parsed = parseLine(line);
        if (parsed.has_value()) {
            result.append(*parsed);
        }
    }
    return result;
}

void SerialTelemetryParser::clear()
{
    m_buffer.clear();
}

std::optional<RawSerialTelemetry> SerialTelemetryParser::parseLine(const QString &line) const
{
    if (!line.startsWith(QStringLiteral("TEL,"))) {
        return std::nullopt;
    }

    const qsizetype separator = line.indexOf(';');
    if (separator < 0 || separator != line.lastIndexOf(';')) {
        return std::nullopt;
    }

    const QStringList fields = line.mid(4, separator - 4).split(',');
    if (fields.size() != 3) {
        return std::nullopt;
    }

    bool rpmOk = false;
    bool batteryOk = false;
    bool errorOk = false;
    bool checksumOk = false;
    const int rpm = fields.at(0).toInt(&rpmOk);
    const double batteryVoltage = fields.at(1).toDouble(&batteryOk);
    const int errorCode = fields.at(2).toInt(&errorOk);
    const int checksum = line.mid(separator + 1).toInt(&checksumOk);

    if (!rpmOk || !batteryOk || !errorOk || !checksumOk) {
        return std::nullopt;
    }

    const int expectedChecksum = (rpm + static_cast<int>(batteryVoltage) + errorCode) & 0xFF;
    if (checksum != expectedChecksum) {
        return std::nullopt;
    }

    return RawSerialTelemetry{rpm, batteryVoltage, errorCode};
}

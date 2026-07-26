#include "DriveModeViewModel.h"

DriveModeViewModel::DriveModeViewModel(QObject *parent) : QObject(parent) {}

QString DriveModeViewModel::driveMode() const { return m_driveMode; }

QString DriveModeViewModel::driveModeLabel() const
{
    if (m_driveMode == QLatin1String("eco"))
        return QStringLiteral("ECO");
    if (m_driveMode == QLatin1String("sport"))
        return QStringLiteral("SPORT");
    return QStringLiteral("NORMAL");
}

void DriveModeViewModel::cycleDriveMode()
{
    if (m_driveMode == QLatin1String("normal")) {
        m_driveMode = QStringLiteral("sport");
    } else if (m_driveMode == QLatin1String("sport")) {
        m_driveMode = QStringLiteral("eco");
    } else {
        m_driveMode = QStringLiteral("normal");
    }
    emit driveModeChanged();
}

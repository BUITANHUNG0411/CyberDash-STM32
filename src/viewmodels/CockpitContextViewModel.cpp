#include "CockpitContextViewModel.h"

#include "DriveModeViewModel.h"
#include "ParkingAssistViewModel.h"
#include "SafetyScenarioViewModel.h"
#include "ThemeViewModel.h"
#include "VehicleModeViewModel.h"

namespace {
QString vehicleModeLabelFor(const QString &vehicleMode)
{
    if (vehicleMode == QLatin1String("bike")) {
        return QStringLiteral("BIKE");
    }
    if (vehicleMode == QLatin1String("scooter")) {
        return QStringLiteral("SCOOTER");
    }
    return QStringLiteral("CAR");
}
}

CockpitContextViewModel::CockpitContextViewModel(VehicleModeViewModel *vehicleMode,
                                                   DriveModeViewModel *driveMode,
                                                   ThemeViewModel *theme,
                                                   ParkingAssistViewModel *parkingAssist,
                                                   SafetyScenarioViewModel *safetyScenario,
                                                   QObject *parent)
    : QObject(parent)
    , m_vehicleMode(vehicleMode)
    , m_driveMode(driveMode)
    , m_theme(theme)
    , m_parkingAssist(parkingAssist)
    , m_safetyScenario(safetyScenario)
{
    if (m_vehicleMode) {
        connect(m_vehicleMode, &VehicleModeViewModel::vehicleModeChanged,
                this, &CockpitContextViewModel::synchronize);
    }
    if (m_driveMode) {
        connect(m_driveMode, &DriveModeViewModel::driveModeChanged,
                this, &CockpitContextViewModel::synchronize);
    }
    if (m_theme) {
        connect(m_theme, &ThemeViewModel::isNightChanged,
                this, &CockpitContextViewModel::synchronize);
    }
    if (m_parkingAssist) {
        connect(m_parkingAssist, &ParkingAssistViewModel::criticalProximityChanged,
                this, &CockpitContextViewModel::synchronize);
    }
    if (m_safetyScenario) {
        connect(m_safetyScenario, &SafetyScenarioViewModel::activeChanged,
                this, &CockpitContextViewModel::synchronize);
        connect(m_safetyScenario, &SafetyScenarioViewModel::canStartChanged,
                this, &CockpitContextViewModel::synchronize);
    }

    synchronize();
}

QString CockpitContextViewModel::vehicleModeLabel() const { return m_vehicleModeLabel; }

QString CockpitContextViewModel::driveModeLabel() const { return m_driveModeLabel; }

QString CockpitContextViewModel::themeLabel() const { return m_themeLabel; }

QString CockpitContextViewModel::sourceLabel() const { return m_sourceLabel; }

QString CockpitContextViewModel::safetyLabLabel() const { return m_safetyLabLabel; }

bool CockpitContextViewModel::safetyLabAvailable() const { return m_safetyLabAvailable; }

void CockpitContextViewModel::setHardwareConnected(bool connected)
{
    if (m_hardwareConnected == connected) {
        return;
    }

    m_hardwareConnected = connected;
    synchronize();
}

void CockpitContextViewModel::synchronize()
{
    const QString vehicleMode = m_vehicleMode ? m_vehicleMode->vehicleMode() : QStringLiteral("car");
    const QString vehicleModeLabel = vehicleModeLabelFor(vehicleMode);
    const QString driveModeLabel = m_driveMode ? m_driveMode->driveModeLabel() : QStringLiteral("NORMAL");
    const QString themeLabel = m_theme && !m_theme->isNight()
        ? QStringLiteral("DAY")
        : QStringLiteral("NIGHT");
    const QString sourceLabel = m_hardwareConnected
        ? QStringLiteral("UART LIVE")
        : QStringLiteral("SIMULATOR");
    const bool carMode = vehicleMode == QLatin1String("car");
    const bool parkingCritical = m_parkingAssist && m_parkingAssist->criticalProximity();
    const bool safetyActive = m_safetyScenario && m_safetyScenario->isActive();
    const bool safetyLabAvailable = carMode && !parkingCritical
        && m_safetyScenario && m_safetyScenario->canStart();
    const QString safetyLabLabel = !carMode
        ? QStringLiteral("LAB: CAR ONLY")
        : parkingCritical
          ? QStringLiteral("LAB: PARKING")
          : safetyActive
            ? QStringLiteral("LAB ACTIVE")
            : safetyLabAvailable
              ? QStringLiteral("LAB READY")
              : QStringLiteral("LAB UNAVAILABLE");

    const bool vehicleModeChanged = m_vehicleModeLabel != vehicleModeLabel;
    const bool driveModeChanged = m_driveModeLabel != driveModeLabel;
    const bool themeChanged = m_themeLabel != themeLabel;
    const bool sourceChanged = m_sourceLabel != sourceLabel;
    const bool safetyLabChanged = m_safetyLabLabel != safetyLabLabel;
    const bool safetyAvailabilityChanged = m_safetyLabAvailable != safetyLabAvailable;

    m_vehicleModeLabel = vehicleModeLabel;
    m_driveModeLabel = driveModeLabel;
    m_themeLabel = themeLabel;
    m_sourceLabel = sourceLabel;
    m_safetyLabLabel = safetyLabLabel;
    m_safetyLabAvailable = safetyLabAvailable;

    if (vehicleModeChanged) {
        emit vehicleModeLabelChanged();
    }
    if (driveModeChanged) {
        emit driveModeLabelChanged();
    }
    if (themeChanged) {
        emit themeLabelChanged();
    }
    if (sourceChanged) {
        emit sourceLabelChanged();
    }
    if (safetyLabChanged) {
        emit safetyLabLabelChanged();
    }
    if (safetyAvailabilityChanged) {
        emit safetyLabAvailableChanged();
    }
}

#include "VehicleModeViewModel.h"

VehicleModeViewModel::VehicleModeViewModel(QObject *parent) : QObject(parent) {}

QString VehicleModeViewModel::vehicleMode() const { return m_vehicleMode; }

void VehicleModeViewModel::cycleVehicleMode()
{
    if (m_vehicleMode == QLatin1String("car")) {
        m_vehicleMode = QStringLiteral("bike");
    } else if (m_vehicleMode == QLatin1String("bike")) {
        m_vehicleMode = QStringLiteral("scooter");
    } else {
        m_vehicleMode = QStringLiteral("car");
    }
    emit vehicleModeChanged();
}

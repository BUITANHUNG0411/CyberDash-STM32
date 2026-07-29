#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class DriveModeViewModel;
class ParkingAssistViewModel;
class SafetyScenarioViewModel;
class ThemeViewModel;
class VehicleModeViewModel;

class CockpitContextViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString vehicleModeLabel READ vehicleModeLabel NOTIFY vehicleModeLabelChanged)
    Q_PROPERTY(QString driveModeLabel READ driveModeLabel NOTIFY driveModeLabelChanged)
    Q_PROPERTY(QString themeLabel READ themeLabel NOTIFY themeLabelChanged)
    Q_PROPERTY(QString sourceLabel READ sourceLabel NOTIFY sourceLabelChanged)
    Q_PROPERTY(QString safetyLabLabel READ safetyLabLabel NOTIFY safetyLabLabelChanged)
    Q_PROPERTY(bool safetyLabAvailable READ safetyLabAvailable NOTIFY safetyLabAvailableChanged)

public:
    explicit CockpitContextViewModel(VehicleModeViewModel *vehicleMode,
                                     DriveModeViewModel *driveMode,
                                     ThemeViewModel *theme,
                                     ParkingAssistViewModel *parkingAssist,
                                     SafetyScenarioViewModel *safetyScenario,
                                     QObject *parent = nullptr);

    QString vehicleModeLabel() const;
    QString driveModeLabel() const;
    QString themeLabel() const;
    QString sourceLabel() const;
    QString safetyLabLabel() const;
    bool safetyLabAvailable() const;

    void setHardwareConnected(bool connected);

signals:
    void vehicleModeLabelChanged();
    void driveModeLabelChanged();
    void themeLabelChanged();
    void sourceLabelChanged();
    void safetyLabLabelChanged();
    void safetyLabAvailableChanged();

private:
    void synchronize();

    QPointer<VehicleModeViewModel> m_vehicleMode;
    QPointer<DriveModeViewModel> m_driveMode;
    QPointer<ThemeViewModel> m_theme;
    QPointer<ParkingAssistViewModel> m_parkingAssist;
    QPointer<SafetyScenarioViewModel> m_safetyScenario;
    bool m_hardwareConnected = false;
    QString m_vehicleModeLabel;
    QString m_driveModeLabel;
    QString m_themeLabel;
    QString m_sourceLabel;
    QString m_safetyLabLabel;
    bool m_safetyLabAvailable = false;
};

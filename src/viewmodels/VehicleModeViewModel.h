#pragma once

#include <QObject>
#include <QString>

class VehicleModeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString vehicleMode READ vehicleMode NOTIFY vehicleModeChanged)

public:
    explicit VehicleModeViewModel(QObject *parent = nullptr);

    QString vehicleMode() const;

    Q_INVOKABLE void cycleVehicleMode();

signals:
    void vehicleModeChanged();

private:
    QString m_vehicleMode = QStringLiteral("car");
};

#pragma once

#include <QObject>
#include <QString>

class DriveModeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString driveMode READ driveMode NOTIFY driveModeChanged)
    Q_PROPERTY(QString driveModeLabel READ driveModeLabel NOTIFY driveModeChanged)

public:
    explicit DriveModeViewModel(QObject *parent = nullptr);

    QString driveMode() const;
    QString driveModeLabel() const;

    Q_INVOKABLE void cycleDriveMode();

signals:
    void driveModeChanged();

private:
    QString m_driveMode = QStringLiteral("normal");
};

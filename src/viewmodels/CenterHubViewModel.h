#pragma once

#include <QObject>

class ParkingAssistViewModel;

class CenterHubViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activePage READ activePage NOTIFY activePageChanged)

public:
    enum Page {
        MusicPage = 0,
        ParkingPage = 1
    };
    Q_ENUM(Page)

    explicit CenterHubViewModel(ParkingAssistViewModel *parkingAssist,
                                QObject *parent = nullptr);

    int activePage() const;

signals:
    void activePageChanged();

private:
    void updateActivePage();

    ParkingAssistViewModel *m_parkingAssist;
    int m_activePage = MusicPage;
};

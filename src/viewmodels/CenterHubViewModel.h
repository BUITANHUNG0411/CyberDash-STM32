#pragma once

#include <QObject>

class ParkingAssistViewModel;

class CenterHubViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activePage READ activePage NOTIFY activePageChanged)

public:
    enum Page : int {
        MusicPage = 0,
        ParkingPage = 1,
    };
    Q_ENUM(Page)

    explicit CenterHubViewModel(ParkingAssistViewModel *parkingAssist,
                                QObject *parent = nullptr);

    int activePage() const;

    Q_INVOKABLE bool selectPage(int page);
    Q_INVOKABLE void setSwipeActive(bool active);
    Q_INVOKABLE void updateSwipeTranslation(qreal translationX);

signals:
    void activePageChanged();

private:
    void updateActivePage();

    ParkingAssistViewModel *m_parkingAssist;
    int m_requestedPage = MusicPage;
    int m_activePage = MusicPage;
    bool m_swipeActive = false;
    qreal m_swipeTranslation = 0.0;
};

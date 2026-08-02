#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class ParkingAssistViewModel;

class CenterHubViewModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int activePage READ activePage NOTIFY activePageChanged)
    Q_PROPERTY(int pageCount READ pageCount CONSTANT)
    Q_PROPERTY(QString activePageLabel READ activePageLabel NOTIFY activePageChanged)

public:
    enum Page : int {
        MusicPage = 0,
        ParkingPage = 1,
        TripPage = 2,
    };
    Q_ENUM(Page)

    explicit CenterHubViewModel(ParkingAssistViewModel *parkingAssist,
                                QObject *parent = nullptr);

    int activePage() const;
    int pageCount() const;
    QString activePageLabel() const;

    Q_INVOKABLE bool selectPage(int page);
    Q_INVOKABLE bool moveSelection(int direction);
    Q_INVOKABLE void setSwipeActive(bool active);
    Q_INVOKABLE void updateSwipeTranslation(qreal translationX);

signals:
    void activePageChanged();

private:
    bool parkingOverrideActive() const;
    void updateActivePage();

    QPointer<ParkingAssistViewModel> m_parkingAssist;
    int m_requestedPage = MusicPage;
    int m_activePage = MusicPage;
    bool m_swipeActive = false;
    qreal m_swipeTranslation = 0.0;
};

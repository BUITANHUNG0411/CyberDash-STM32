#pragma once

#include <QObject>
#include <QPointer>

class QSequentialAnimationGroup;

class ThemeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isNight READ isNight NOTIFY isNightChanged)
    Q_PROPERTY(int bootStage READ bootStage NOTIFY bootStageChanged)
    Q_PROPERTY(bool isBooting READ isBooting NOTIFY bootStageChanged)
    Q_PROPERTY(qreal bootProgress READ bootProgress NOTIFY bootProgressChanged)

public:
    explicit ThemeViewModel(int sweepLegDurationMs = 900, QObject *parent = nullptr);

    bool isNight() const;
    int bootStage() const;
    bool isBooting() const;
    qreal bootProgress() const;

    Q_INVOKABLE void toggleTheme();
    Q_INVOKABLE void handleWindowDragActive(bool active);
    void startBootSequence();

signals:
    void isNightChanged();
    void bootStageChanged();
    void bootProgressChanged();
    void windowMoveRequested();

private:
    void setBootStage(int stage);
    void setBootProgress(qreal progress);

    bool m_isNight = true;
    int m_bootStage = 0;
    qreal m_bootProgress = 0.0;
    int m_sweepLegDurationMs;
    QPointer<QSequentialAnimationGroup> m_bootTimeline;
};

#pragma once

#include <QObject>

class ThemeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isNight READ isNight NOTIFY isNightChanged)

public:
    explicit ThemeViewModel(QObject *parent = nullptr);

    bool isNight() const;

    Q_INVOKABLE void toggleTheme();
    Q_INVOKABLE void handleWindowDragActive(bool active);

signals:
    void isNightChanged();
    void windowMoveRequested();

private:
    bool m_isNight = true;
};

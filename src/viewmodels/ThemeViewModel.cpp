#include "ThemeViewModel.h"

ThemeViewModel::ThemeViewModel(QObject *parent) : QObject(parent) {}

bool ThemeViewModel::isNight() const { return m_isNight; }

void ThemeViewModel::toggleTheme()
{
    m_isNight = !m_isNight;
    emit isNightChanged();
}

void ThemeViewModel::handleWindowDragActive(bool active)
{
    if (active) {
        emit windowMoveRequested();
    }
}

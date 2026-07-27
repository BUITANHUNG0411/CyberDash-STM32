#include "ThemeViewModel.h"

#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>

ThemeViewModel::ThemeViewModel(int sweepLegDurationMs, QObject *parent)
    : QObject(parent), m_sweepLegDurationMs(sweepLegDurationMs)
{
}

bool ThemeViewModel::isNight() const { return m_isNight; }
int ThemeViewModel::bootStage() const { return m_bootStage; }
bool ThemeViewModel::isBooting() const { return m_bootStage < 2; }
qreal ThemeViewModel::bootProgress() const { return m_bootProgress; }

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

void ThemeViewModel::startBootSequence()
{
    if (m_bootTimeline) {
        return; // already started (idempotent)
    }

    auto *timeline = new QSequentialAnimationGroup(this);
    m_bootTimeline = timeline;

    const auto addLeg = [this, timeline](qreal from, qreal to) {
        auto *leg = new QVariantAnimation(timeline);
        leg->setStartValue(from);
        leg->setEndValue(to);
        leg->setDuration(m_sweepLegDurationMs);
        leg->setEasingCurve(QEasingCurve::InOutQuad);
        connect(leg, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) { setBootProgress(value.toReal()); });
        timeline->addAnimation(leg);
    };

    addLeg(0.0, 1.0);
    addLeg(1.0, 0.0);

    connect(timeline, &QSequentialAnimationGroup::finished, this,
            [this]() { setBootStage(2); });

    setBootStage(1);
    timeline->start();
}

void ThemeViewModel::setBootStage(int stage)
{
    if (m_bootStage == stage) {
        return;
    }
    m_bootStage = stage;
    emit bootStageChanged();
}

void ThemeViewModel::setBootProgress(qreal progress)
{
    if (qFuzzyCompare(m_bootProgress, progress)) {
        return;
    }
    m_bootProgress = progress;
    emit bootProgressChanged();
}

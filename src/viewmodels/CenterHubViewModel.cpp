#include "CenterHubViewModel.h"

#include "ParkingAssistViewModel.h"

#include <QtGlobal>

namespace {
constexpr qreal swipeCommitDistance = 80.0;
}

CenterHubViewModel::CenterHubViewModel(ParkingAssistViewModel *parkingAssist, QObject *parent)
    : QObject(parent), m_parkingAssist(parkingAssist)
{
    if (!m_parkingAssist) {
        return;
    }

    connect(m_parkingAssist, &ParkingAssistViewModel::criticalProximityChanged,
            this, &CenterHubViewModel::updateActivePage);
    updateActivePage();
}

int CenterHubViewModel::activePage() const { return m_activePage; }

bool CenterHubViewModel::selectPage(int page)
{
    if (page != MusicPage && page != ParkingPage) {
        return false;
    }

    if (page != ParkingPage && m_parkingAssist && m_parkingAssist->criticalProximity()) {
        return false;
    }

    m_requestedPage = page;
    if (m_activePage != page) {
        m_activePage = page;
        emit activePageChanged();
    }
    return true;
}

void CenterHubViewModel::setSwipeActive(bool active)
{
    if (active) {
        m_swipeActive = true;
        m_swipeTranslation = 0.0;
        return;
    }

    if (!m_swipeActive) {
        return;
    }

    m_swipeActive = false;
    const qreal translation = m_swipeTranslation;
    m_swipeTranslation = 0.0;

    if (translation <= -swipeCommitDistance) {
        selectPage(ParkingPage);
    } else if (translation >= swipeCommitDistance) {
        selectPage(MusicPage);
    }
}

void CenterHubViewModel::updateSwipeTranslation(qreal translationX)
{
    if (m_swipeActive && qIsFinite(translationX)) {
        m_swipeTranslation = translationX;
    }
}

void CenterHubViewModel::updateActivePage()
{
    const int newActivePage = m_parkingAssist && m_parkingAssist->criticalProximity()
        ? ParkingPage
        : m_requestedPage;
    if (m_activePage != newActivePage) {
        m_activePage = newActivePage;
        emit activePageChanged();
    }
}

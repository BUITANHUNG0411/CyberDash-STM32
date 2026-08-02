#include "CenterHubViewModel.h"

#include "ParkingAssistViewModel.h"

#include <QtGlobal>

namespace {
constexpr qreal swipeCommitDistance = 80.0;
constexpr int firstPage = CenterHubViewModel::MusicPage;
constexpr int lastPage = CenterHubViewModel::TripPage;
constexpr int previousPageDirection = -1;
constexpr int nextPageDirection = 1;
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

int CenterHubViewModel::pageCount() const { return lastPage - firstPage + 1; }

QString CenterHubViewModel::activePageLabel() const
{
    switch (m_activePage) {
    case MusicPage:
        return QStringLiteral("MUSIC");
    case ParkingPage:
        return QStringLiteral("PARK");
    case TripPage:
        return QStringLiteral("TRIP");
    default:
        return QStringLiteral("MUSIC");
    }
}

bool CenterHubViewModel::parkingOverrideActive() const
{
    return m_parkingAssist && m_parkingAssist->criticalProximity();
}

bool CenterHubViewModel::selectPage(int page)
{
    if (page < firstPage || page > lastPage) {
        return false;
    }

    if (page != ParkingPage && parkingOverrideActive()) {
        return false;
    }

    m_requestedPage = page;
    if (m_activePage != page) {
        m_activePage = page;
        emit activePageChanged();
    }
    return true;
}

bool CenterHubViewModel::moveSelection(int direction)
{
    if (direction != previousPageDirection && direction != nextPageDirection) {
        return false;
    }

    const int currentNavigationPage = parkingOverrideActive()
        ? m_activePage
        : m_requestedPage;
    const int candidatePage = currentNavigationPage + direction;
    if (candidatePage < firstPage || candidatePage > lastPage) {
        return false;
    }

    return selectPage(candidatePage);
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
    const int newActivePage = parkingOverrideActive()
        ? ParkingPage
        : m_requestedPage;
    if (m_activePage != newActivePage) {
        m_activePage = newActivePage;
        emit activePageChanged();
    }
}

#include "CenterHubViewModel.h"

#include "ParkingAssistViewModel.h"

CenterHubViewModel::CenterHubViewModel(ParkingAssistViewModel *parkingAssist, QObject *parent)
    : QObject(parent), m_parkingAssist(parkingAssist)
{
    Q_ASSERT(m_parkingAssist);
    connect(m_parkingAssist, &ParkingAssistViewModel::reverseActiveChanged,
            this, &CenterHubViewModel::updateActivePage);
    updateActivePage();
}

int CenterHubViewModel::activePage() const { return m_activePage; }

void CenterHubViewModel::updateActivePage()
{
    const int newActivePage = m_parkingAssist->reverseActive() ? ParkingPage : MusicPage;
    if (m_activePage == newActivePage) {
        return;
    }

    m_activePage = newActivePage;
    emit activePageChanged();
}

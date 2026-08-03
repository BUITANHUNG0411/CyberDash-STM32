pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import com.showcase

/*
 * Hub trung tâm (Car mode): giữ MusicPlayer sống ổn định trong panel.
 */
Item {
    id: root

    readonly property color activePageAccent: CenterHubController.activePage === 1
                                               ? ParkingAssist.proximityLevel === 3
                                                 ? Theme.warningRed
                                                 : Theme.parkingCaution
                                               : Theme.accentCyan

    StackLayout {
        currentIndex: CenterHubController.activePage
        anchors {
            fill: parent
            topMargin: Theme.centerStatusLedHeight
                        + Theme.spaceLg
                        + Theme.centerNavTabHeight
                        + Theme.spaceSm
        }

        MusicPlayer {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ParkingAssistView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        TripComputerView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }

    Row {
        id: clusterStatusLeds
        spacing: Theme.centerStatusLedSpacing
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: Theme.spaceXs
        }

        Rectangle {
            width: Theme.centerStatusLedWidth
            height: Theme.centerStatusLedHeight
            radius: height / 2
            color: CenterHubController.activePage === 0
                   ? Theme.accentCyan
                   : Theme.trackInactive
        }

        Rectangle {
            width: Theme.centerStatusLedWidth
            height: Theme.centerStatusLedHeight
            radius: height / 2
            color: CenterHubController.activePage === 1
                   ? root.activePageAccent
                   : Theme.trackInactive
        }

        Rectangle {
            width: Theme.centerStatusLedWidth
            height: Theme.centerStatusLedHeight
            radius: height / 2
            color: CenterHubController.activePage === 2
                   ? Theme.accentCyan
                   : Theme.trackInactive
        }
    }

    RowLayout {
        spacing: Theme.spaceSm
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: Theme.centerStatusLedHeight + Theme.spaceLg
        }

        Rectangle {
            radius: Theme.radiusSm
            color: CenterHubController.activePage === 0 ? root.activePageAccent : Theme.trackInactive
            border.color: CenterHubController.activePage === 0
                          ? root.activePageAccent
                          : Theme.glassPanelBorder
            border.width: 1
            Layout.preferredWidth: Theme.centerNavTabWidth
            Layout.preferredHeight: Theme.centerNavTabHeight

            Text {
                anchors.centerIn: parent
                text: "MUSIC"
                color: CenterHubController.activePage === 0 ? Theme.textOnAccent : Theme.textSecondary
                font { family: Theme.fontMain; pixelSize: Theme.textXs; bold: true }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: CenterHubController.selectPage(0)
            }
        }

        Rectangle {
            radius: Theme.radiusSm
            color: CenterHubController.activePage === 1 ? root.activePageAccent : Theme.trackInactive
            border.color: CenterHubController.activePage === 1
                          ? root.activePageAccent
                          : Theme.glassPanelBorder
            border.width: 1
            Layout.preferredWidth: Theme.centerNavTabWidth
            Layout.preferredHeight: Theme.centerNavTabHeight

            Text {
                anchors.centerIn: parent
                text: "PARK"
                color: CenterHubController.activePage === 1 ? Theme.textOnAccent : Theme.textSecondary
                font { family: Theme.fontMain; pixelSize: Theme.textXs; bold: true }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: CenterHubController.selectPage(1)
            }
        }

        Rectangle {
            radius: Theme.radiusSm
            color: CenterHubController.activePage === 2 ? root.activePageAccent : Theme.trackInactive
            border.color: CenterHubController.activePage === 2
                          ? root.activePageAccent
                          : Theme.glassPanelBorder
            border.width: 1
            Layout.preferredWidth: Theme.centerNavTabWidth
            Layout.preferredHeight: Theme.centerNavTabHeight

            Text {
                anchors.centerIn: parent
                text: "TRIP"
                color: CenterHubController.activePage === 2 ? Theme.textOnAccent : Theme.textSecondary
                font { family: Theme.fontMain; pixelSize: Theme.textXs; bold: true }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: CenterHubController.selectPage(2)
            }
        }

    }

    // The hub owns horizontal navigation; child MouseAreas still receive clicks,
    // while a real drag can take over and is committed by the C++ ViewModel.
    DragHandler {
        id: hubSwipeHandler
        target: null
        grabPermissions: PointerHandler.CanTakeOverFromAnything

        onActiveChanged: CenterHubController.setSwipeActive(active)
        onTranslationChanged: CenterHubController.updateSwipeTranslation(translation.x)
    }
}

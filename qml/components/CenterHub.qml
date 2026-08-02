pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import com.showcase

/*
 * Hub trung tâm (Car mode): giữ MusicPlayer sống ổn định trong panel.
 */
Item {
    id: root

    StackLayout {
        currentIndex: CenterHubController.activePage
        anchors {
            fill: parent
            topMargin: Theme.spaceXl + 24 + Theme.spaceSm
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

    RowLayout {
        spacing: Theme.spaceSm
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            topMargin: Theme.spaceMd
        }

        Rectangle {
            radius: Theme.radiusSm
            color: CenterHubController.activePage === 0 ? Theme.accentCyan : Theme.trackInactive
            border.color: Theme.glassPanelBorder
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
            color: CenterHubController.activePage === 1 ? Theme.accentCyan : Theme.trackInactive
            border.color: Theme.glassPanelBorder
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
            color: CenterHubController.activePage === 2 ? Theme.accentCyan : Theme.trackInactive
            border.color: Theme.glassPanelBorder
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

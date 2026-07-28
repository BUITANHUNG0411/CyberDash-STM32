import QtQuick
import com.showcase

Item {
    id: root

    visible: ParkingAssist.reverseActive

    GlassPanel {
        anchors.fill: parent
    }

    Rectangle {
        id: bumper
        width: parent.width * 0.68
        height: parent.height * 0.18
        radius: height / 2
        color: Theme.glassPanelBase
        border.width: 1
        border.color: Theme.glassPanelBorder
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Theme.spaceXl
        }

        Rectangle {
            width: parent.width * 0.22
            height: 3
            anchors.centerIn: parent
            radius: height / 2
            color: Theme.accentCyan
        }
    }

    Text {
        id: distanceText
        color: Theme.textPrimary
        text: ParkingAssist.distanceText
        font {
            family: Theme.fontDisplay
            pixelSize: Theme.textXl
            bold: true
        }
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: Theme.spaceXl
        }
    }

    Text {
        color: Theme.textSecondary
        text: ParkingAssist.statusText
        font {
            family: Theme.fontMain
            pixelSize: Theme.textXs
            letterSpacing: 1.5
        }
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: distanceText.bottom
            topMargin: Theme.spaceSm
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spaceMd

        Rectangle {
            width: root.width * 0.30
            height: Theme.spaceSm
            radius: height / 2
            color: ParkingAssist.proximityLevel === 3
                   ? Theme.warningRed
                   : ParkingAssist.proximityLevel === 2
                     ? Theme.parkingCaution
                     : ParkingAssist.proximityLevel === 1
                       ? Theme.accentCyan
                       : Theme.parkingUnavailable
            opacity: ParkingAssist.proximityLevel === 1 ? 1.0 : 0.32
        }

        Rectangle {
            width: root.width * 0.46
            height: Theme.spaceSm
            radius: height / 2
            color: ParkingAssist.proximityLevel === 3
                   ? Theme.warningRed
                   : ParkingAssist.proximityLevel === 2
                     ? Theme.parkingCaution
                     : ParkingAssist.proximityLevel === 1
                       ? Theme.accentCyan
                       : Theme.parkingUnavailable
            opacity: ParkingAssist.proximityLevel === 2 ? 1.0 : 0.32
        }

        Rectangle {
            width: root.width * 0.62
            height: Theme.spaceSm
            radius: height / 2
            color: ParkingAssist.proximityLevel === 3
                   ? Theme.warningRed
                   : ParkingAssist.proximityLevel === 2
                     ? Theme.parkingCaution
                     : ParkingAssist.proximityLevel === 1
                       ? Theme.accentCyan
                       : Theme.parkingUnavailable
            opacity: ParkingAssist.proximityLevel === 3 ? 1.0 : 0.32
        }
    }
}

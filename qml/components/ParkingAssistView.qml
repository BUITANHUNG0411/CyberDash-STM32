import QtQuick
import com.showcase

Item {
    id: root

    property real displayedDistanceCm: ParkingAssist.rearDistanceCm
    readonly property bool stopActive: ParkingAssist.proximityLevel === 3
    readonly property color proximityColor: ParkingAssist.proximityLevel === 3
                                           ? Theme.warningRed
                                           : ParkingAssist.proximityLevel === 2
                                             ? Theme.parkingCaution
                                             : ParkingAssist.proximityLevel === 1
                                               ? Theme.accentCyan
                                               : Theme.parkingUnavailable

    Behavior on displayedDistanceCm {
        NumberAnimation {
            duration: Theme.durationNormal
            easing.type: Easing.OutCubic
        }
    }

    GlassPanel {
        anchors.fill: parent
    }

    Column {
        id: header
        spacing: Theme.spaceXs
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.spaceXl
            rightMargin: Theme.spaceXl
            top: parent.top
            topMargin: Theme.spaceXl
        }

        Text {
            width: parent.width
            color: Theme.textPrimary
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            text: "REAR PARK ASSIST"
            font {
                family: Theme.fontMain
                pixelSize: Theme.textXs
                letterSpacing: 1.8
            }
        }

        Text {
            width: parent.width
            color: ParkingAssist.sensorHealth === 1
                   ? Theme.accentCyan
                   : ParkingAssist.sensorHealth === 2
                     ? Theme.parkingCaution
                     : Theme.parkingUnavailable
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            text: ParkingAssist.healthText
            font {
                family: Theme.fontMain
                pixelSize: Theme.textXs
                letterSpacing: 1.1
            }
        }
    }

    Text {
        id: distanceReadout
        color: Theme.textPrimary
        text: ParkingAssist.sensorAvailable ? ParkingAssist.formatDistance(root.displayedDistanceCm) : "—"
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: header.bottom
            topMargin: Theme.spaceLg
        }
        font {
            family: Theme.fontDisplay
            pixelSize: Theme.displayMd
            bold: true
        }
    }

    Text {
        id: statusLabel
        color: root.proximityColor
        text: ParkingAssist.statusText
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: distanceReadout.bottom
        }
        font {
            family: Theme.fontMain
            pixelSize: Theme.textSm
            letterSpacing: 2.0
        }
    }

    Item {
        id: sensorZone
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.spaceXXl
            rightMargin: Theme.spaceXXl
            top: statusLabel.bottom
            topMargin: Theme.spaceXl
            bottom: segmentTrack.top
            bottomMargin: Theme.spaceXl
        }

        Rectangle {
            id: obstacleBlock
            width: sensorZone.width * 0.16
            height: Theme.spaceLg
            radius: Theme.radiusSm
            x: (sensorZone.width - width) / 2
            y: ParkingAssist.proximityProgress * (sensorZone.height - height - Theme.spaceLg)
            color: root.proximityColor
            opacity: ParkingAssist.sensorAvailable ? 1.0 : 0.0

            Behavior on y {
                NumberAnimation {
                    duration: Theme.durationNormal
                    easing.type: Easing.OutCubic
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.durationFast
                }
            }
        }

        Rectangle {
            id: bumperLine
            width: parent.width
            height: Theme.spaceXs
            radius: height / 2
            anchors.bottom: parent.bottom
            color: root.proximityColor
            opacity: ParkingAssist.sensorAvailable ? 0.78 : 0.34

            Behavior on color {
                ColorAnimation {
                    duration: Theme.durationNormal
                }
            }
        }

        Rectangle {
            anchors.fill: bumperLine
            radius: bumperLine.radius
            color: Theme.warningRed
            opacity: 0.0

            SequentialAnimation on opacity {
                running: root.stopActive
                loops: Animation.Infinite

                NumberAnimation {
                    to: 1.0
                    duration: Theme.durationNormal
                }

                NumberAnimation {
                    to: 0.0
                    duration: Theme.durationNormal
                }
            }
        }
    }

    Row {
        id: segmentTrack
        spacing: Theme.spaceSm
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Theme.spaceXl
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 1 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 2 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 3 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 4 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 5 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 6 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 7 ? 1.0 : 0.24
        }

        Rectangle {
            width: (root.width - Theme.spaceXXl * 2 - Theme.spaceSm * 7) / 8
            height: Theme.spaceMd
            radius: height / 2
            color: root.proximityColor
            opacity: ParkingAssist.proximitySegments >= 8 ? 1.0 : 0.24
        }
    }
}

import QtQuick
import com.showcase

Item {
    id: root

    property color severityColor: SafetyScenario.criticalActive
                                  ? Theme.warningRed
                                  : SafetyScenario.advisoryActive
                                    ? Theme.parkingCaution
                                    : Theme.accentCyan

    visible: opacity > 0.0
    opacity: SafetyScenario.presentationVisible ? 1.0 : 0.0
    scale: SafetyScenario.presentationVisible ? 1.0 : 0.96

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.durationNormal
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: Theme.durationNormal
            easing.type: Easing.OutCubic
        }
    }

    GlassPanel {
        anchors.fill: parent
    }

    Row {
        id: header
        spacing: Theme.spaceMd
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: Theme.spaceXl
            leftMargin: Theme.spaceXl
            rightMargin: Theme.spaceXl
        }

        Column {
            width: parent.width - exitLab.width - parent.spacing
            spacing: Theme.spaceXs

            Text {
                width: parent.width
                color: root.severityColor
                horizontalAlignment: Text.AlignLeft
                text: SafetyScenario.title
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textMd
                    weight: Font.DemiBold
                    letterSpacing: 0.8
                }
            }

            Text {
                width: parent.width
                color: Theme.textPrimary
                horizontalAlignment: Text.AlignLeft
                text: SafetyScenario.instructionText
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                    letterSpacing: 0.4
                }
            }
        }

        Rectangle {
            id: exitLab
            width: Theme.spaceXXl + Theme.spaceLg
            height: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.glassPanelBase
            border.color: Theme.glassPanelBorder
            border.width: 1

            Text {
                anchors.centerIn: parent
                color: Theme.textSecondary
                text: "EXIT"
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                    letterSpacing: 0.3
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: SafetyScenario.stopDemo()
            }
        }
    }

    Item {
        id: threatZone
        anchors {
            top: header.bottom
            bottom: riskTrack.top
            left: parent.left
            right: parent.right
            topMargin: Theme.spaceXl
            bottomMargin: Theme.spaceXl
            leftMargin: Theme.spaceXl
            rightMargin: Theme.spaceXl
        }

        Rectangle {
            height: Theme.spaceXs
            radius: Theme.radiusPill
            color: Theme.trackInactive
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            id: threatGate
            width: Theme.spaceXl + Theme.spaceXs
            height: Theme.spaceXXl + Theme.spaceXl
            radius: Theme.radiusSm
            x: SafetyScenario.threatPosition * (threatZone.width - width)
            anchors.verticalCenter: parent.verticalCenter
            color: root.severityColor
            opacity: SafetyScenario.pulseActive ? 1.0 : 0.72
            scale: SafetyScenario.pulseActive ? 1.1 : 1.0

            Behavior on x {
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

            Behavior on scale {
                NumberAnimation {
                    duration: Theme.durationFast
                }
            }

            Rectangle {
                radius: Theme.radiusSm
                color: Theme.glassPanelBase
                border.color: root.severityColor
                border.width: 1
                anchors {
                    fill: parent
                    margins: Theme.spaceXs
                }
            }
        }
    }

    Row {
        id: riskTrack
        spacing: Theme.spaceSm
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: acknowledgement.top
            bottomMargin: Theme.spaceLg
        }

        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 1 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 2 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 3 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 4 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 5 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 6 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 7 ? 1.0 : 0.24 }
        Rectangle { width: Theme.spaceXl; height: Theme.spaceMd; radius: Theme.radiusPill; color: root.severityColor; opacity: SafetyScenario.riskSegments >= 8 ? 1.0 : 0.24 }
    }

    Rectangle {
        id: acknowledgement
        width: Theme.displayMd + Theme.spaceXl
        height: Theme.spaceXXl
        radius: Theme.radiusPill
        color: Theme.glassPanelBase
        border.color: root.severityColor
        border.width: 1
        visible: SafetyScenario.acknowledgementAvailable
        enabled: SafetyScenario.acknowledgementAvailable
        opacity: SafetyScenario.acknowledgementAvailable ? 1.0 : 0.36
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: disclaimer.top
            bottomMargin: Theme.spaceLg
        }

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.durationFast
            }
        }

        Text {
            anchors.centerIn: parent
            color: root.severityColor
            text: "ACKNOWLEDGE"
            font {
                family: Theme.fontMain
                pixelSize: Theme.textXs
                letterSpacing: 1.4
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: SafetyScenario.acknowledgementAvailable
            onClicked: SafetyScenario.acknowledge()
        }
    }

    Text {
        id: disclaimer
        color: Theme.textSecondary
        text: SafetyScenario.disclaimerText
        font {
            family: Theme.fontMain
            pixelSize: Theme.textXs
            letterSpacing: 0.7
        }
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Theme.spaceXl
        }
    }
}

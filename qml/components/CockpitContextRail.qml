import QtQuick
import QtQuick.Layouts
import com.showcase

Item {
    id: root

    implicitWidth: contextRow.implicitWidth + Theme.spaceXXl
    implicitHeight: Theme.spaceXl + Theme.spaceMd
    opacity: ThemeController.bootStage < 2 ? 0.0 : 1.0
    visible: opacity > 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.durationSlow
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusPill
        color: Theme.glassPanelBase
        border.color: Theme.glassPanelBorder
        border.width: 1
    }

    RowLayout {
        id: contextRow
        anchors.centerIn: parent
        spacing: Theme.spaceSm

        Rectangle {
            implicitWidth: vehicleModeText.implicitWidth + Theme.spaceXl
            implicitHeight: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.trackInactive

            Text {
                id: vehicleModeText
                anchors.centerIn: parent
                color: Theme.accentCyan
                text: CockpitContext.vehicleModeLabel
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                }
            }
        }

        Rectangle {
            implicitWidth: driveModeText.implicitWidth + Theme.spaceXl
            implicitHeight: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.trackInactive

            Text {
                id: driveModeText
                anchors.centerIn: parent
                color: Theme.accentCyan
                text: CockpitContext.driveModeLabel
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                }
            }
        }

        Rectangle {
            implicitWidth: themeText.implicitWidth + Theme.spaceXl
            implicitHeight: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.trackInactive

            Text {
                id: themeText
                anchors.centerIn: parent
                color: Theme.textPrimary
                text: CockpitContext.themeLabel
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                }
            }
        }

        Rectangle {
            implicitWidth: sourceText.implicitWidth + Theme.spaceXl
            implicitHeight: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.trackInactive

            Text {
                id: sourceText
                anchors.centerIn: parent
                color: Theme.textPrimary
                text: CockpitContext.sourceLabel
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                }
            }
        }

        Rectangle {
            implicitWidth: safetyLabText.implicitWidth + Theme.spaceXl
            implicitHeight: Theme.spaceXl
            radius: Theme.radiusPill
            color: Theme.trackInactive

            Text {
                id: safetyLabText
                anchors.centerIn: parent
                color: CockpitContext.safetyLabAvailable ? Theme.accentCyan : Theme.parkingUnavailable
                text: CockpitContext.safetyLabLabel
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textXs
                }
            }
        }
    }
}

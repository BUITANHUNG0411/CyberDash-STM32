import QtQuick
import QtQuick.Layouts
import com.showcase

GlassPanel {
    id: root

    ColumnLayout {
        spacing: Theme.spaceLg
        anchors {
            fill: parent
            leftMargin: Theme.spaceXl
            rightMargin: Theme.spaceXl
            topMargin: Theme.spaceXl
            bottomMargin: Theme.spaceXl
        }

        Text {
            text: "TRIP COMPUTER"
            color: Theme.accentCyan
            Layout.alignment: Qt.AlignHCenter
            font {
                family: Theme.fontMain
                pixelSize: Theme.textMd
                bold: true
                letterSpacing: 2
            }
        }

        RowLayout {
            spacing: Theme.spaceLg
            Layout.fillWidth: true

            ColumnLayout {
                spacing: Theme.spaceSm
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: "TRIP"
                    color: Theme.textSecondary
                    Layout.alignment: Qt.AlignHCenter
                    font {
                        family: Theme.fontMain
                        pixelSize: Theme.textSm
                        letterSpacing: 1
                    }
                }

                GlowingText {
                    text: TripComputer.tripDisplay
                    color: Theme.textPrimary
                    glowColor: Theme.accentCyan
                    font.pixelSize: Theme.displaySm
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            ColumnLayout {
                spacing: Theme.spaceSm
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: "ODO"
                    color: Theme.textSecondary
                    Layout.alignment: Qt.AlignHCenter
                    font {
                        family: Theme.fontMain
                        pixelSize: Theme.textSm
                        letterSpacing: 1
                    }
                }

                GlowingText {
                    text: TripComputer.odoDisplay
                    color: Theme.textPrimary
                    glowColor: Theme.accentCyan
                    font.pixelSize: Theme.displaySm
                    Layout.alignment: Qt.AlignHCenter
                }
            }

        }

        ColumnLayout {
            spacing: Theme.spaceSm
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Text {
                text: "AVG SPEED"
                color: Theme.textSecondary
                Layout.alignment: Qt.AlignHCenter
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textSm
                    letterSpacing: 1
                }
            }

            GlowingText {
                text: TripComputer.avgSpeedDisplay
                color: Theme.textPrimary
                glowColor: Theme.accentCyan
                font.pixelSize: Theme.displaySm
                Layout.alignment: Qt.AlignHCenter
            }
        }

        Text {
            text: "CTRL + LEFT / RIGHT TO NAVIGATE"
            color: Theme.textSecondary
            Layout.alignment: Qt.AlignHCenter
            font {
                family: Theme.fontMain
                pixelSize: Theme.textXs
                letterSpacing: 1
            }
        }
    }
}

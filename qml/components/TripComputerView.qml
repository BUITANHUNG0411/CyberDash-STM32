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

        RowLayout {
            spacing: Theme.spaceMd
            Layout.fillWidth: true

            Text {
                color: Theme.accentCyan
                text: "TRIP COMPUTER"
                Layout.fillWidth: true
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.textMd
                    bold: true
                    letterSpacing: 2
                }
            }

            Text {
                color: Theme.textSecondary
                text: "DRIVE DATA"
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.tripMetaDisplay
                    letterSpacing: 1.2
                }
            }
        }

        Rectangle {
            color: Theme.accentCyan
            opacity: 0.38
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        ColumnLayout {
            spacing: Theme.spaceSm
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Text {
                color: Theme.textSecondary
                text: "SESSION DISTANCE"
                Layout.alignment: Qt.AlignHCenter
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.tripMetaDisplay
                    letterSpacing: 1.6
                }
            }

            GlowingText {
                id: tripHero
                text: TripComputer.tripDisplay
                color: Theme.textPrimary
                glowColor: Theme.accentCyan
                font.pixelSize: Theme.tripHeroDisplay
                Layout.alignment: Qt.AlignHCenter
            }

            Rectangle {
                radius: height / 2
                color: Theme.accentCyan
                opacity: 0.78
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Theme.tripRailWidth
                Layout.preferredHeight: 3
            }
        }

        RowLayout {
            id: statsRow
            spacing: Theme.spaceMd
            Layout.fillWidth: true

            Rectangle {
                color: Theme.trackInactive
                border.color: Theme.glassPanelBorder
                border.width: 1
                radius: Theme.radiusSm
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.tripCardHeight

                ColumnLayout {
                    spacing: Theme.spaceXs
                    anchors.fill: parent
                    anchors.margins: Theme.spaceMd

                    Text {
                        color: Theme.textSecondary
                        text: "ODOMETER"
                        Layout.fillWidth: true
                        font {
                            family: Theme.fontMain
                            pixelSize: Theme.tripMetaDisplay
                            letterSpacing: 1.4
                        }
                    }

                    GlowingText {
                        text: TripComputer.odoDisplay
                        color: Theme.textPrimary
                        glowColor: Theme.accentCyan
                        font.pixelSize: Theme.tripMetricDisplay
                        Layout.fillWidth: true
                    }
                }
            }

            Rectangle {
                color: Theme.trackInactive
                border.color: Theme.glassPanelBorder
                border.width: 1
                radius: Theme.radiusSm
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.tripCardHeight

                ColumnLayout {
                    spacing: Theme.spaceXs
                    anchors.fill: parent
                    anchors.margins: Theme.spaceMd

                    Text {
                        color: Theme.textSecondary
                        text: "AVG SPEED"
                        Layout.fillWidth: true
                        font {
                            family: Theme.fontMain
                            pixelSize: Theme.tripMetaDisplay
                            letterSpacing: 1.4
                        }
                    }

                    GlowingText {
                        text: TripComputer.avgSpeedDisplay
                        color: Theme.textPrimary
                        glowColor: Theme.accentCyan
                        font.pixelSize: Theme.tripMetricDisplay
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: Theme.spaceMd
        }

        RowLayout {
            spacing: Theme.spaceMd
            Layout.fillWidth: true

            Rectangle {
                color: Theme.accentCyan
                opacity: 0.25
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 2
            }

            Text {
                color: Theme.textSecondary
                text: "DRIVE SESSION"
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.tripMetaDisplay
                    letterSpacing: 1.2
                }
            }

            Rectangle {
                color: Theme.accentCyan
                opacity: 0.25
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 2
            }
        }

        RowLayout {
            spacing: Theme.spaceMd
            Layout.fillWidth: true

            Text {
                color: Theme.textSecondary
                text: "CTRL + LEFT / RIGHT TO NAVIGATE"
                Layout.fillWidth: true
                font {
                    family: Theme.fontMain
                    pixelSize: Theme.tripMetaDisplay
                    letterSpacing: 0.9
                }
            }

            Rectangle {
                color: Theme.trackInactive
                border.color: Theme.accentCyan
                border.width: 1
                radius: Theme.radiusSm
                Layout.preferredWidth: Theme.tripResetWidth
                Layout.preferredHeight: Theme.tripResetHeight

                Text {
                    anchors.centerIn: parent
                    color: Theme.accentCyan
                    text: "RESET TRIP"
                    font {
                        family: Theme.fontMain
                        pixelSize: Theme.tripMetaDisplay
                        bold: true
                        letterSpacing: 0.8
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: TripComputer.resetTrip()
                }
            }
        }
    }
}

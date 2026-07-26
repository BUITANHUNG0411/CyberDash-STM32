import QtQuick
import QtQuick.Layouts
import com.showcase
import "../components"

Item {
    id: root
    anchors.fill: parent

    property QtObject vm: VehicleStatus

    // Top Bar (Telltales)
    RowLayout {
        id: topBar
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 70
        spacing: Theme.spaceXXl

        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/warning-triangle.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: vm.isWarning ? Theme.warningRed : Theme.textSecondary
            opacity: vm.isWarning ? 1.0 : 0.2
        }
        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/battery-low.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: vm.battery < 20 ? Theme.warningRed : Theme.textSecondary
            opacity: vm.battery < 20 ? 1.0 : 0.2
        }
        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/temperature-high.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: vm.temperature > 85 ? Theme.warningRed : Theme.textSecondary
            opacity: vm.temperature > 85 ? 1.0 : 0.2
        }
    }

    // Main 3-Panel Layout
    Item {
        anchors.fill: parent

        // Left Panel (Speed)
        Item {
            id: leftPanel
            width: 350
            height: 400
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -Theme.panelLift
            
            // Replaced absolute x with anchors linked to Theme values
            anchors.left: parent.left
            anchors.leftMargin: Theme.gaugeInsetLeft - width / 2

            NeonTickGauge {
                anchors.centerIn: parent
                width: 350
                height: 350
                value: vm.speed
                maxValue: 160
                isWarning: vm.isWarning
                tickCount: 80
                majorTickInterval: 10
            }

            Column {
                anchors.centerIn: parent
                spacing: 0
                
                GlowingText {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: vm.displaySpeed
                    font.pixelSize: Theme.displayMd
                    glowColor: vm.isWarning ? Theme.warningRed : Theme.accentCyan
                    color: Theme.textPrimary
                }
                
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "KM/H"
                    color: Theme.textSecondary
                    font.family: Theme.fontMain
                    font.pixelSize: Theme.textMd
                    font.letterSpacing: 2
                }
            }
        }

        // Center Panel (Media/Nav)
        // centerPanel: intentional z:-1 inset between the two gauge bezels.
        // DO NOT convert to horizontalCenter — it would break the depth layering.
        Item {
            id: centerPanel
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -Theme.panelLift
            height: 450
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.leftMargin: -20
            anchors.rightMargin: -20
            z: -1 // Push slightly behind the gauges to create depth and prevent overlapping the glowing ticks

            MusicPlayer {
                anchors.fill: parent
                anchors.margins: 10
            }
        }

        // Right Panel (Gear/RPM)
        Item {
            id: rightPanel
            width: 350
            height: 400
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -Theme.panelLift
            
            // Replaced absolute x with anchors linked to Theme values
            anchors.left: parent.left
            anchors.leftMargin: Theme.gaugeInsetRight - width / 2

            NeonTickGauge {
                anchors.centerIn: parent
                width: 350
                height: 350
                value: vm.rpm
                maxValue: 6000
                isWarning: vm.isWarning
                tickCount: 60
                majorTickInterval: 10
                redlineValue: 5000
            }

            Column {
                anchors.centerIn: parent
                spacing: 10
                
                GlowingText {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: vm.gear
                    font.pixelSize: Theme.displayLg
                    glowColor: vm.isWarning ? Theme.warningRed : Theme.accentCyan
                    color: Theme.textPrimary
                }
                
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "COMFORT"
                    color: Theme.accentCyan
                    font.family: Theme.fontMain
                    font.pixelSize: Theme.textSm
                    font.letterSpacing: 2
                }
            }
        }
    }

    // Bottom Bar (Battery, Range, Temp)
    RowLayout {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 80
        width: parent.width * 0.75
        
        // Left: Range
        Column {
            Layout.alignment: Qt.AlignLeft
            spacing: Theme.spaceMd
            Text { 
                text: vm.range + " km"
                color: Theme.textPrimary
                font.family: Theme.fontMain
                font.pixelSize: Theme.textLg 
            }
            EnergyBlocks {
                count: 20
                activeCount: 15
            }
        }

        Item { Layout.fillWidth: true } // Spacer

        // Center: Temp
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spaceSm
            Text { 
                text: "TEMP"
                color: Theme.textSecondary
                font.family: Theme.fontMain
                font.pixelSize: Theme.textXs
                font.letterSpacing: 1
                anchors.horizontalCenter: parent.horizontalCenter 
            }
            Text { 
                text: vm.temperature + " °C"
                color: Theme.textPrimary
                font.family: Theme.fontMain
                font.pixelSize: Theme.textXl
                anchors.horizontalCenter: parent.horizontalCenter 
            }
        }

        Item { Layout.fillWidth: true } // Spacer

        // Right: Battery
        Column {
            Layout.alignment: Qt.AlignRight
            spacing: Theme.spaceMd
            Text { 
                text: vm.battery + " %"
                color: Theme.textPrimary
                font.family: Theme.fontMain
                font.pixelSize: Theme.textLg
                anchors.right: parent.right 
            }
            EnergyBlocks {
                anchors.right: parent.right 
                count: 20
                activeCount: vm.battery / 5
                dangerThreshold: 4
            }
        }
    }
}

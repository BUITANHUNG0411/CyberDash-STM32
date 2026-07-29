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
            colorizationColor: ThemeController.isBooting ? Theme.warningRed
                               : (vm.isWarning ? Theme.warningRed : Theme.textSecondary)
            opacity: ThemeController.isBooting ? 1.0 : (vm.isWarning ? 1.0 : 0.2)
        }
        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/battery-low.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: ThemeController.isBooting ? Theme.warningRed
                               : (vm.battery < 20 ? Theme.warningRed : Theme.textSecondary)
            opacity: ThemeController.isBooting ? 1.0 : (vm.battery < 20 ? 1.0 : 0.2)
        }
        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/temperature-high.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: ThemeController.isBooting ? Theme.warningRed
                               : (vm.temperature > 85 ? Theme.warningRed : Theme.textSecondary)
            opacity: ThemeController.isBooting ? 1.0 : (vm.temperature > 85 ? 1.0 : 0.2)
        }

        // Theme toggle — icon shows the mode you will switch TO
        NeonIconButton {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: ThemeController.isNight
                    ? "qrc:/qt/qml/com/showcase/resources/icons/theme-day.svg"
                    : "qrc:/qt/qml/com/showcase/resources/icons/theme-night.svg"
            sourceSize: Qt.size(28, 28)
            defaultColor: Theme.textSecondary
            onClicked: ThemeController.toggleTheme()
        }

        // Vehicle morph cycle — icon shows the mode you will switch TO
        NeonIconButton {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            enabled: !ThemeController.isBooting
            opacity: enabled ? 1.0 : 0.4
            source: VehicleMode.vehicleMode === "car"
                    ? "qrc:/qt/qml/com/showcase/resources/icons/vehicle-bike.svg"
                    : VehicleMode.vehicleMode === "bike"
                      ? "qrc:/qt/qml/com/showcase/resources/icons/vehicle-scooter.svg"
                      : "qrc:/qt/qml/com/showcase/resources/icons/vehicle-car.svg"
            sourceSize: Qt.size(28, 28)
            defaultColor: Theme.textSecondary
            onClicked: VehicleMode.cycleVehicleMode()
        }

        // Drive mode cycle — icon shows the mode you will switch TO
        NeonIconButton {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: DriveMode.driveMode === "normal"
                    ? "qrc:/qt/qml/com/showcase/resources/icons/drive-sport.svg"
                    : DriveMode.driveMode === "sport"
                      ? "qrc:/qt/qml/com/showcase/resources/icons/drive-eco.svg"
                      : "qrc:/qt/qml/com/showcase/resources/icons/drive-normal.svg"
            sourceSize: Qt.size(28, 28)
            defaultColor: Theme.textSecondary
            onClicked: DriveMode.cycleDriveMode()
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
                id: speedGauge
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
        // centerPanel stays behind the gauge faces, with a tokenized clear gap
        // on both sides to avoid the central surface touching their tick rings.
        Item {
            id: centerPanel
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -Theme.panelLift
            height: 450
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.leftMargin: Theme.centerPanelGap
            anchors.rightMargin: Theme.centerPanelGap
            z: -1 // Push slightly behind the gauges to create depth and prevent overlapping the glowing ticks
            opacity: ThemeController.bootStage < 2 ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: Theme.durationSlow } }

            CenterHub {
                id: centerHub
                anchors.fill: parent
                anchors.margins: 10
                opacity: 1
                visible: opacity > 0
            }

            // Card Range/Trip cho Scooter — Loader tự unload sau khi fade xong (Zero-JS)
            Loader {
                id: scooterCard
                anchors.fill: parent
                anchors.margins: Theme.spaceXXl
                asynchronous: true
                opacity: 0
                visible: opacity > 0
                active: VehicleMode.vehicleMode === "scooter" || opacity > 0
                source: "../components/RangeTripCard.qml"
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

            // Nhóm gauge RPM/Battery + chữ giữa — ẩn nguyên khối ở chế độ Bike
            Item {
                id: rightGaugeGroup
                anchors.fill: parent
                visible: opacity > 0

                NeonTickGauge {
                    id: rightGauge
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
                        id: gearText
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: vm.gear
                        font.pixelSize: Theme.displayLg
                        glowColor: vm.isWarning ? Theme.warningRed : Theme.accentCyan
                        color: Theme.textPrimary
                    }

                    Text {
                        id: gearSubText
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: DriveMode.driveModeLabel
                        color: Theme.accentCyan
                        font.family: Theme.fontMain
                        font.pixelSize: Theme.textSm
                        font.letterSpacing: 2
                    }
                }
            }

            // Hiển thị % pin lớn cho chế độ Bike (thay gauge phải)
            Column {
                id: bikeBatteryDisplay
                anchors.centerIn: parent
                spacing: 0
                opacity: 0
                visible: opacity > 0
                scale: 0.85

                GlowingText {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: vm.battery + "%"
                    font.pixelSize: Theme.displayLg
                    glowColor: vm.battery < 20 ? Theme.warningRed : Theme.accentCyan
                    color: Theme.textPrimary
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "BATTERY"
                    color: Theme.textSecondary
                    font.family: Theme.fontMain
                    font.pixelSize: Theme.textMd
                    font.letterSpacing: 2
                }
            }
        }
    }

    // Bottom Bar (Battery, Range, Temp)
    RowLayout {
        id: bottomBar
        opacity: ThemeController.bootStage < 2 ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: Theme.durationSlow } }
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
            Text {
                text: "TRIP " + TripComputer.tripDisplay + "  ·  ODO " + TripComputer.odoDisplay
                color: Theme.textSecondary
                font.family: Theme.fontMain
                font.pixelSize: Theme.textXs
                font.letterSpacing: 1
                anchors.horizontalCenter: parent.horizontalCenter

                // Click để reset trip (Zero-JS: một lời gọi Q_INVOKABLE)
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -Theme.spaceMd // nới vùng chạm
                    onClicked: TripComputer.resetTrip()
                }
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

    // ── Vehicle Morphing: State-Driven Layouts (Decision Log) ──
    // State "car" rỗng = base bindings chính là layout Car.
    // PropertyChanges tự khôi phục binding gốc khi rời state.
    state: VehicleMode.vehicleMode

    states: [
        State { name: "car" },

        State {
            name: "scooter"
            PropertyChanges {
                target: speedGauge
                maxValue: 120
                tickCount: 60
                majorTickInterval: 10
            }
            PropertyChanges {
                target: rightGauge
                value: vm.battery
                maxValue: 100
                tickCount: 50
                majorTickInterval: 10
                redlineValue: -1
            }
            PropertyChanges { target: gearText; text: vm.battery }
            PropertyChanges { target: gearSubText; text: "BATT %" }
            PropertyChanges { target: centerHub; opacity: 0 }
            PropertyChanges { target: scooterCard; opacity: 1 }
        },

        State {
            name: "bike"
            PropertyChanges {
                target: speedGauge
                maxValue: 60
                tickCount: 60
                majorTickInterval: 10
            }
            PropertyChanges { target: rightGaugeGroup; opacity: 0 }
            PropertyChanges { target: bikeBatteryDisplay; opacity: 1; scale: 1.0 }
            PropertyChanges { target: centerHub; opacity: 0 }
            PropertyChanges { target: bottomBar; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            SequentialAnimation {
                // Pha A — "dip": chìm 2 vòm gauge để che khoảnh khắc relabel tick
                ParallelAnimation {
                    NumberAnimation { targets: [leftPanel, rightPanel]; property: "opacity"; to: 0.0; duration: Theme.durationNormal; easing.type: Easing.InQuad }
                    NumberAnimation { targets: [leftPanel, rightPanel]; property: "scale"; to: 0.94; duration: Theme.durationNormal; easing.type: Easing.InQuad }
                }
                // Pha B — áp thay đổi tức thời khi đang ẩn
                PropertyAction { targets: [speedGauge, rightGauge]; properties: "maxValue,tickCount,majorTickInterval,redlineValue,value" }
                PropertyAction { targets: [gearText, gearSubText]; property: "text" }
                PropertyAction { targets: [rightGaugeGroup, bikeBatteryDisplay]; property: "opacity" }
                PropertyAction { target: bikeBatteryDisplay; property: "scale" }
                // Pha C — nổi trở lại + cross-fade center/bottom song song
                ParallelAnimation {
                    NumberAnimation { targets: [leftPanel, rightPanel]; property: "opacity"; to: 1.0; duration: Theme.durationSlow; easing.type: Easing.OutCubic }
                    NumberAnimation { targets: [leftPanel, rightPanel]; property: "scale"; to: 1.0; duration: Theme.durationSlow; easing.type: Easing.OutBack }
                    NumberAnimation { targets: [centerHub, scooterCard, bottomBar]; property: "opacity"; duration: Theme.durationSlow; easing.type: Easing.OutQuad }
                }
            }
        }
    ]
}

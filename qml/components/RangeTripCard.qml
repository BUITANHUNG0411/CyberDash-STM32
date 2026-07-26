import QtQuick
import com.showcase

/*
 * Thẻ Range/Trip cho chế độ Scooter — thay thế MusicPlayer ở panel trung tâm.
 * Thuần hiển thị (Zero-JS): bind trực tiếp vào VehicleStatus.
 */
GlassPanel {
    id: root
    property QtObject vm: VehicleStatus

    Column {
        anchors.centerIn: parent
        spacing: Theme.spaceXl

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "RANGE"
            color: Theme.textSecondary
            font.family: Theme.fontMain
            font.pixelSize: Theme.textSm
            font.letterSpacing: 2
        }
        GlowingText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.vm.range + " km"
            font.pixelSize: Theme.displayMd * 0.6
            glowColor: Theme.accentCyan
            color: Theme.textPrimary
        }
        EnergyBlocks {
            anchors.horizontalCenter: parent.horizontalCenter
            count: 20
            activeCount: root.vm.battery / 5
            dangerThreshold: 4
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.vm.battery + " %   •   " + root.vm.temperature + " °C"
            color: Theme.textPrimary
            font.family: Theme.fontMain
            font.pixelSize: Theme.textLg
        }
    }
}

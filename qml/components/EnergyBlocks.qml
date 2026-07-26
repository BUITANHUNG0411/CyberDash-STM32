import QtQuick
import QtQuick.Effects
import com.showcase

Item {
    id: root
    property int count: 20
    property int activeCount: 15
    property color activeColor: Theme.accentCyan
    property color inactiveColor: Theme.textSecondary
    property color dangerColor: Theme.warningRed
    property int dangerThreshold: 4

    implicitWidth: blockRow.implicitWidth
    implicitHeight: blockRow.implicitHeight

    Row {
        id: blockRow
        spacing: Theme.spaceXs
        Repeater {
            model: root.count
            Rectangle {
                width: 8; height: 8; radius: 2
                property bool isActive: index < root.activeCount
                color: isActive ? (root.activeCount < root.dangerThreshold ? root.dangerColor : root.activeColor) : root.inactiveColor
                opacity: isActive ? 1.0 : 0.3
            }
        }
    }
}

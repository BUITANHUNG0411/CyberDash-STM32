import QtQuick
import com.showcase

Rectangle {
    id: root

    property color accentColor: Theme.accentCyan

    color: Theme.glassPanelBase
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd

    InstrumentFrame {
        anchors.fill: parent
        accentColor: root.accentColor
        z: 10
    }
}

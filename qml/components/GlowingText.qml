import QtQuick
import QtQuick.Effects
import com.showcase

Item {
    id: root
    property alias text: label.text
    property alias font: label.font
    property alias color: label.color
    property color glowColor: Theme.accentCyan
    property real glowRadius: Theme.radiusSm

    implicitWidth: label.implicitWidth
    implicitHeight: label.implicitHeight

    Text {
        id: label
        anchors.centerIn: parent
        color: Theme.textPrimary
        font.family: Theme.fontDisplay
        font.pixelSize: Theme.textXl
        font.bold: true
        visible: false // Hidden because MultiEffect will render it
    }

    MultiEffect {
        source: label
        anchors.fill: label
        shadowEnabled: true
        shadowColor: root.glowColor
        shadowBlur: root.glowRadius
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 0
    }
}

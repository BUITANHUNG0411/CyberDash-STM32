import QtQuick
import QtQuick.Effects
import com.showcase

Item {
    id: root
    property alias source: iconImg.source
    property alias sourceSize: iconImg.sourceSize
    property color colorizationColor: Theme.textSecondary

    Behavior on opacity { NumberAnimation { duration: Theme.durationNormal } }

    Image {
        id: iconImg
        anchors.fill: parent
        visible: false
    }
    
    MultiEffect {
        anchors.fill: iconImg
        source: iconImg
        colorization: 1.0
        colorizationColor: root.colorizationColor
        Behavior on colorizationColor { ColorAnimation { duration: Theme.durationFast } }
    }
}

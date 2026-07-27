import QtQuick
import QtQuick.Shapes
import com.showcase

Item {
    id: root

    property real directionDegrees: 0.0

    implicitWidth: Theme.encoderVehicleWidth
    implicitHeight: Theme.encoderVehicleHeight
    anchors.centerIn: parent
    rotation: root.directionDegrees
    transformOrigin: Item.Center

    Rectangle {
        x: 32
        y: 82
        width: 32
        height: 36
        radius: 16
        color: Theme.arrowShadow
        opacity: 0.55
    }

    Shape {
        ShapePath {
            strokeColor: Theme.arrowHighlight
            strokeWidth: 2
            fillColor: Theme.accentCyan

            PathSvg {
                path: "M 36 74 L 68 18 L 100 74 H 82 V 108 H 54 V 74 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.arrowFrame
            strokeWidth: 1.5
            fillColor: "transparent"

            PathSvg {
                path: "M 52 75 L 68 47 L 84 75"
            }
        }
    }

    Rectangle {
        x: 61
        y: 56
        width: 14
        height: 30
        radius: 7
        color: Theme.textOnAccent
        opacity: 0.35
    }

    Behavior on rotation {
        NumberAnimation {
            duration: Theme.durationGauge
            easing.type: Easing.OutQuad
        }
    }
}

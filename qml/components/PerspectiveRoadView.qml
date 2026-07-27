import QtQuick
import QtQuick.Shapes
import com.showcase

Rectangle {
    id: root
    color: Theme.glassPanelBase
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd
    clip: true

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.45
        height: parent.height * 0.18
        radius: height / 2
        color: Theme.roadHorizonGlow
        opacity: 0.75
    }

    Repeater {
        model: RoadMotion

        Shape {
            required property real leftNearX
            required property real rightNearX
            required property real leftFarX
            required property real rightFarX
            required property real nearY
            required property real farY
            required property real centerNearX
            required property real centerFarX
            required property real centerNearY
            required property real centerFarY
            required property bool centerLineVisible
            required property real segmentOpacity
            required property real segmentDepth

            anchors.fill: parent
            opacity: segmentOpacity
            z: segmentDepth

            ShapePath {
                strokeWidth: 0
                fillColor: Theme.roadSurface
                startX: leftFarX * root.width
                startY: farY * root.height
                PathLine { x: rightFarX * root.width; y: farY * root.height }
                PathLine { x: rightNearX * root.width; y: nearY * root.height }
                PathLine { x: leftNearX * root.width; y: nearY * root.height }
                PathLine { x: leftFarX * root.width; y: farY * root.height }
            }

            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 2
                fillColor: "transparent"
                startX: leftFarX * root.width
                startY: farY * root.height
                PathLine { x: leftNearX * root.width; y: nearY * root.height }
            }

            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 2
                fillColor: "transparent"
                startX: rightFarX * root.width
                startY: farY * root.height
                PathLine { x: rightNearX * root.width; y: nearY * root.height }
            }

            ShapePath {
                strokeColor: centerLineVisible
                    ? Theme.roadLaneMarker
                    : "transparent"
                strokeWidth: 2
                fillColor: "transparent"
                startX: centerFarX * root.width
                startY: centerFarY * root.height
                PathLine {
                    x: centerNearX * root.width
                    y: centerNearY * root.height
                }
            }
        }
    }

    Shape {
        width: 28
        height: 38
        z: 100
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Theme.spaceXl
        }

        ShapePath {
            strokeColor: Theme.accentCyan
            strokeWidth: 2
            fillColor: Theme.textPrimary
            startX: 14
            startY: 0
            PathLine { x: 28; y: 38 }
            PathLine { x: 14; y: 29 }
            PathLine { x: 0; y: 38 }
            PathLine { x: 14; y: 0 }
        }
    }
}

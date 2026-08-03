import QtQuick
import QtQuick.Shapes
import com.showcase

Item {
    id: root

    property color accentColor: Theme.accentCyan

    Shape {
        id: outerFrame
        anchors.fill: parent

        ShapePath {
            fillColor: "transparent"
            strokeColor: Theme.glassPanelBorder
            strokeWidth: Theme.clusterFrameLineWidth
            startX: 0
            startY: 0

            PathLine { x: outerFrame.width; y: 0 }
            PathLine { x: outerFrame.width; y: outerFrame.height }
            PathLine { x: 0; y: outerFrame.height }
            PathLine { x: 0; y: 0 }
        }
    }

    Shape {
        id: innerFrame
        anchors.fill: parent
        anchors.margins: Theme.clusterFrameInset

        ShapePath {
            fillColor: "transparent"
            strokeColor: Theme.glassEdge
            strokeWidth: Theme.clusterFrameLineWidth
            startX: 0
            startY: 0

            PathLine { x: innerFrame.width; y: 0 }
            PathLine { x: innerFrame.width; y: innerFrame.height }
            PathLine { x: 0; y: innerFrame.height }
            PathLine { x: 0; y: 0 }
        }
    }

    Rectangle {
        x: Theme.clusterFrameInset
        y: Theme.clusterFrameInset
        width: parent.width * Theme.clusterFrameHighlightRatio
        height: Theme.clusterFrameHighlightHeight
        radius: height / 2
        color: root.accentColor
        opacity: Theme.clusterFrameHighlightOpacity
    }
}

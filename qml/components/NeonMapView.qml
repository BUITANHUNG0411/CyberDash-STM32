import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import com.showcase

/*
 * Bản đồ neon cách điệu (Zero-JS). Marker chạy theo quãng đường thật:
 * PathInterpolator.progress bind vào MapModel.routeProgress (C++).
 */
Rectangle {
    id: root
    color: Theme.glassPanelBase
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd
    clip: true

    // Không gian thiết kế cố định 400×360, scale khít khung
    Item {
        id: mapCanvas
        width: 400
        height: 360
        anchors.centerIn: parent
        scale: root.width / width < root.height / height
            ? root.width / width
            : root.height / height

        // Lưới phố nền
        Repeater {
            model: 9
            Rectangle {
                required property int index
                x: 40 + index * 40
                width: 1
                height: mapCanvas.height
                color: Theme.textSecondary
                opacity: 0.08
            }
        }
        Repeater {
            model: 8
            Rectangle {
                required property int index
                y: 40 + index * 40
                width: mapCanvas.width
                height: 1
                color: Theme.textSecondary
                opacity: 0.08
            }
        }

        // Phố phụ (nét xám)
        Shape {
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4
            ShapePath {
                strokeColor: Theme.textSecondary
                strokeWidth: 3
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                PathSvg { path: "M 20 120 L 380 120 M 260 20 L 260 340 M 20 220 L 200 220" }
            }
        }

        // Route chính (neon accent — tự đổi màu theo drive mode)
        Shape {
            id: routeShape
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4
            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 5
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                PathSvg { path: "M 80 60 L 320 60 Q 340 60 340 80 L 340 160 Q 340 180 320 180 L 220 180 Q 200 180 200 200 L 200 260 Q 200 280 180 280 L 100 280 Q 80 280 80 260 L 80 80 Q 80 60 100 60 Z" }
            }
        }

        // Neon bloom cho route (source là sibling — đúng quy tắc MultiEffect)
        MultiEffect {
            anchors.fill: routeShape
            source: routeShape
            shadowEnabled: true
            shadowColor: Theme.accentCyan
            shadowBlur: 1.0
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }

        // Vị trí + hướng dọc route, điều khiển từ C++
        PathInterpolator {
            id: routeInterp
            progress: MapModel.routeProgress
            path: Path {
                startX: 80; startY: 60
                PathSvg { path: "L 320 60 Q 340 60 340 80 L 340 160 Q 340 180 320 180 L 220 180 Q 200 180 200 200 L 200 260 Q 200 280 180 280 L 100 280 Q 80 280 80 260 L 80 80 Q 80 60 100 60 Z" }
            }
        }

        // Marker mũi tên
        Item {
            id: marker
            x: routeInterp.x
            y: routeInterp.y
            rotation: routeInterp.angle + 90

            Behavior on x { NumberAnimation { duration: Theme.durationGauge } }
            Behavior on y { NumberAnimation { duration: Theme.durationGauge } }

            Shape {
                anchors.centerIn: parent
                width: 18
                height: 22
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    strokeWidth: 0
                    fillColor: Theme.textPrimary
                    startX: 9; startY: 0
                    PathLine { x: 18; y: 22 }
                    PathLine { x: 9; y: 16 }
                    PathLine { x: 0; y: 22 }
                    PathLine { x: 9; y: 0 }
                }
            }
        }
    }

    // Nhãn TRIP góc trên (tái dùng chuỗi format từ C++)
    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Theme.spaceMd
        text: "TRIP " + TripComputer.tripDisplay
        color: Theme.textSecondary
        font.family: Theme.fontMain
        font.pixelSize: Theme.textXs
        font.letterSpacing: 1
    }
}

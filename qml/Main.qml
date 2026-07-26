import QtQuick
import QtQuick.Shapes
import com.showcase

Window {
    id: root
    width: 1200
    height: 800
    visible: true
    title: qsTr("QtStmAutomotiveSimulator")
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    // Use the backend-injected ViewModel
    property QtObject vm: VehicleStatus

    Shortcut {
        sequence: "Esc"
        onActivated: Qt.quit()
    }

    // Khung Cluster vật lý giả lập dạng Double Arch (Dựa theo tài liệu inspiration-design)
    Item {
        id: clusterFrame
        anchors.fill: parent
        anchors.margins: Theme.bezelMargin // Tạo không gian nền trong suốt để nổi bật khối

        Shape {
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 8 // Chống răng cưa (anti-aliasing) chất lượng cao

            // Nền chính của khung
            ShapePath {
                fillColor: Theme.backgroundDeepSpace
                strokeColor: Theme.bezelStroke
                strokeWidth: 2

                // Đường dẫn SVG tinh chỉnh cho mượt hoàn hảo bằng Cubic Bezier (C/S)
                PathSvg {
                    path: "M 270 0 " +
                          "C 420 0, 480 50, 580 50 " +
                          "S 740 0, 890 0 " +
                          "A 250 380 0 0 1 890 760 " +
                          "C 740 760, 680 710, 580 710 " +
                          "S 420 760, 270 760 " +
                          "A 250 380 0 0 1 270 0 Z"
                }
            }

            // Lớp thứ 2: Glass Reflection (Viền sáng mô phỏng mép kính vát)
            ShapePath {
                fillColor: "transparent"
                strokeColor: Theme.glassEdge
                strokeWidth: 4

                // Thu nhỏ (inset) 4 pixel so với viền ngoài
                PathSvg {
                    path: "M 270 4 " +
                          "C 420 4, 480 54, 580 54 " +
                          "S 740 4, 890 4 " +
                          "A 246 376 0 0 1 886 756 " +
                          "C 740 756, 680 706, 580 706 " +
                          "S 420 756, 270 756 " +
                          "A 246 376 0 0 1 274 4 Z"
                }
            }
        }

        // Kéo thả cửa sổ: CHỈ ở dải bezel trên cùng (như title bar).
        // Không gắn DragHandler lên cả clusterFrame — nó cướp grab của
        // Flickable/SwipeView bên trong, làm gesture vuốt kéo cả cửa sổ theo.
        Item {
            id: windowDragStrip
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60

            DragHandler {
                target: null
                onActiveChanged: active && root.startSystemMove()
            }
        }

        DashboardScreen {
            anchors.fill: parent
            anchors.margins: Theme.dashboardMargin
        }
    }
}

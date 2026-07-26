import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import com.showcase

Item {
    id: root
    width: 300
    height: 300

    property real value: 0
    property real maxValue: 100
    property bool isWarning: false
    property color gaugeColor: Theme.accentCyan
    property color warningColor: Theme.warningRed
    property color effectiveColor: isWarning ? warningColor : gaugeColor

    // Cấu hình số lượng vạch tick
    property int tickCount: 80 
    // Hiển thị vạch lớn (kèm số) mỗi N vạch
    property int majorTickInterval: 10 
    // Điểm bắt đầu vùng đỏ (Redline), nếu > 0 sẽ kích hoạt
    property real redlineValue: -1 

    // Giá trị hiển thị chạy mượt mà (trailing effect).
    // Khi boot: hiển thị sweep 0→max→0 do C++ điều khiển (Behavior tắt để bám sát timeline).
    property real displayedValue: ThemeController.isBooting
                                  ? ThemeController.bootProgress * maxValue
                                  : value
    Behavior on displayedValue {
        enabled: !ThemeController.isBooting
        NumberAnimation { duration: Theme.durationGauge; easing.type: Easing.OutQuad }
    }

    // Lớp 1: Ánh sáng nền (Ambient Backlight)
    Shape {
        anchors.fill: parent
        layer.enabled: true
        layer.samples: 4
        
        ShapePath {
            strokeWidth: 0
            fillGradient: RadialGradient {
                centerX: root.width / 2; centerY: root.height / 2
                centerRadius: root.width / 2.2
                GradientStop { 
                    position: 0.0; 
                    color: Qt.rgba(root.effectiveColor.r, root.effectiveColor.g, root.effectiveColor.b, 0.2) 
                }
                GradientStop { position: 1.0; color: "transparent" }
            }
            startX: 0; startY: 0
            PathLine { x: root.width; y: 0 }
            PathLine { x: root.width; y: root.height }
            PathLine { x: 0; y: root.height }
            PathLine { x: 0; y: 0 }
        }
    }

    // Lớp 2: Vùng chứa các vạch (Ticks Container)
    Item {
        id: tickLayer
        anchors.fill: parent
        
        Repeater {
            model: root.tickCount + 1
            
            Item {
                width: root.width
                height: root.height
                anchors.centerIn: parent
                // Góc quay từ 135 đến 135+270 độ
                rotation: 135 + (index / root.tickCount) * 270

                property real tickValue: (index / root.tickCount) * root.maxValue
                property int displayTickValue: tickValue + 0.5
                property bool isIlluminated: tickValue <= root.displayedValue
                property bool isMajor: index % root.majorTickInterval === 0
                property bool isRedline: root.redlineValue > 0 && tickValue >= root.redlineValue

                Item {
                    anchors.centerIn: parent
                    width: root.width - Theme.spaceXXl
                    height: width
                    
                    Rectangle {
                        property color activeColor: parent.parent.isRedline ? root.warningColor : root.effectiveColor
                        
                        width: parent.parent.isMajor ? root.width * 0.04 : root.width * 0.02
                        height: parent.parent.isMajor ? 4 : 2
                        
                        color: parent.parent.isIlluminated ? (parent.parent.isRedline ? Theme.tickLitMajor : Theme.tickLitMinor) : 
                               (parent.parent.isMajor ? Theme.tickDimMajor : Theme.tickDimMinor)
                        opacity: parent.parent.isIlluminated ? 1.0 : (parent.parent.isMajor ? 0.8 : 0.4)
                        
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        
                        Behavior on opacity { NumberAnimation { duration: Theme.durationTick } }
                        Behavior on color { ColorAnimation { duration: Theme.durationTick } }
                    }
                }

                // Chữ số nhãn
                Item {
                    anchors.centerIn: parent
                    width: root.width - (Theme.spaceXXl * 2.5)
                    height: width
                    
                    Text {
                        visible: parent.parent.isMajor
                        text: parent.parent.displayTickValue
                        color: parent.parent.isIlluminated ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontMain
                        font.pixelSize: root.width * 0.045
                        opacity: parent.parent.isIlluminated ? 1.0 : 0.4
                        
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        // Giữ chữ luôn thẳng đứng
                        rotation: -(135 + (index / root.tickCount) * 270)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        
                        Behavior on opacity { NumberAnimation { duration: Theme.durationTick } }
                    }
                }
            }
        }
    }
    
    // Lớp 3: Hiệu ứng Phát sáng (Neon Bloom)
    MultiEffect {
        anchors.fill: tickLayer
        source: tickLayer
        shadowEnabled: true
        shadowColor: root.effectiveColor
        shadowBlur: 1.0 // Blur tối đa để tạo vệt sáng
        shadowHorizontalOffset: 0
        shadowVerticalOffset: 0
    }
}

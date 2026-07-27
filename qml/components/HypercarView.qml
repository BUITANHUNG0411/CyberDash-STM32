import QtQuick
import QtQuick.Shapes
import com.showcase

Item {
    id: root

    property real lateralOffset: 0.0
    property real yawDegrees: 0.0
    property real frontWheelSteerDegrees: 0.0

    implicitWidth: Theme.encoderVehicleWidth
    implicitHeight: Theme.encoderVehicleHeight
    x: root.lateralOffset
    rotation: root.yawDegrees
    transformOrigin: Item.Bottom

    Rectangle {
        x: 38
        y: 176
        width: 152
        height: 24
        radius: 12
        color: Theme.raceKartShadow
    }

    Item {
        x: 44
        y: 44
        width: 140
        height: 136

        Shape {
            ShapePath {
                strokeColor: Theme.raceKartFrame
                strokeWidth: 5
                fillColor: "transparent"

                PathSvg {
                    path: "M 28 111 L 45 56 Q 69 30 70 14 M 112 111 L 95 56 Q 71 30 70 14 M 35 95 H 105 M 43 63 H 97"
                }
            }
        }

        Shape {
            ShapePath {
                strokeColor: Theme.raceKartHighlight
                strokeWidth: 2
                fillColor: Theme.raceKartBody

                PathSvg {
                    path: "M 36 105 Q 42 78 55 58 Q 63 45 70 42 Q 77 45 85 58 Q 98 78 104 105 L 94 125 H 46 Z"
                }
            }
        }

        Shape {
            ShapePath {
                strokeWidth: 0
                fillColor: Theme.raceKartSeat

                PathSvg {
                    path: "M 52 88 Q 56 62 70 54 Q 84 62 88 88 L 82 106 H 58 Z"
                }
            }
        }

        Shape {
            ShapePath {
                strokeColor: Theme.raceKartFrame
                strokeWidth: 3
                fillColor: "transparent"

                PathSvg {
                    path: "M 56 80 Q 70 69 84 80"
                }
            }
        }

        Rectangle {
            x: 63
            y: 22
            width: 14
            height: 45
            radius: 7
            color: Theme.raceKartFrame
        }

        Rectangle {
            x: 48
            y: 116
            width: 44
            height: 12
            radius: 6
            color: Theme.raceKartFrame
        }

        Rectangle {
            x: 18
            y: 116
            width: 104
            height: 8
            radius: 4
            color: Theme.raceKartFrame
        }

        Item {
            x: 10
            y: 95
            width: 30
            height: 48

            Rectangle {
                anchors.fill: parent
                radius: 9
                color: Theme.raceKartTire
                border.color: Theme.raceKartHighlight
                border.width: 1
            }

            Rectangle {
                x: 10
                y: 17
                width: 10
                height: 14
                radius: 5
                color: Theme.raceKartWheelHub
            }
        }

        Item {
            x: 100
            y: 95
            width: 30
            height: 48

            Rectangle {
                anchors.fill: parent
                radius: 9
                color: Theme.raceKartTire
                border.color: Theme.raceKartHighlight
                border.width: 1
            }

            Rectangle {
                x: 10
                y: 17
                width: 10
                height: 14
                radius: 5
                color: Theme.raceKartWheelHub
            }
        }

        Item {
            x: 15
            y: 12
            width: 28
            height: 46
            rotation: root.frontWheelSteerDegrees
            transformOrigin: Item.Center

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: Theme.raceKartTire
                border.color: Theme.raceKartHighlight
                border.width: 1
            }

            Rectangle {
                x: 9
                y: 16
                width: 10
                height: 14
                radius: 5
                color: Theme.raceKartWheelHub
            }

            Behavior on rotation {
                NumberAnimation {
                    duration: Theme.durationGauge
                    easing.type: Easing.OutQuad
                }
            }
        }

        Item {
            x: 97
            y: 12
            width: 28
            height: 46
            rotation: root.frontWheelSteerDegrees
            transformOrigin: Item.Center

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: Theme.raceKartTire
                border.color: Theme.raceKartHighlight
                border.width: 1
            }

            Rectangle {
                x: 9
                y: 16
                width: 10
                height: 14
                radius: 5
                color: Theme.raceKartWheelHub
            }

            Behavior on rotation {
                NumberAnimation {
                    duration: Theme.durationGauge
                    easing.type: Easing.OutQuad
                }
            }
        }
    }

    Behavior on x {
        NumberAnimation {
            duration: Theme.durationGauge
            easing.type: Easing.OutQuad
        }
    }

    Behavior on rotation {
        NumberAnimation {
            duration: Theme.durationGauge
            easing.type: Easing.OutQuad
        }
    }
}

import QtQuick
import QtQuick.Shapes
import com.showcase

Rectangle {
    id: root

    color: Theme.encoderSky
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd

    Rectangle {
        color: Theme.encoderGround

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: parent.height * Theme.encoderRoadHorizon
            bottomMargin: Theme.radiusMd
            leftMargin: root.border.width
            rightMargin: root.border.width
        }
    }

    Rectangle {
        height: Theme.radiusMd
        color: Theme.encoderGround

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            bottomMargin: root.border.width
            leftMargin: Theme.radiusMd
            rightMargin: Theme.radiusMd
        }
    }

    Rectangle {
        width: parent.width * 0.54
        height: 2
        radius: 1
        color: Theme.roadHorizonGlow

        anchors {
            top: parent.top
            topMargin: parent.height * Theme.encoderRoadHorizon
            horizontalCenter: parent.horizontalCenter
        }
    }

    Shape {
        width: 1
        height: 1

        transform: Scale {
            origin.x: 0
            origin.y: 0
            xScale: root.width
            yScale: root.height
        }

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.roadSurface

            PathSvg {
                path: EncoderDrive.roadPath
            }
        }
    }

    Shape {
        width: 1
        height: 1

        transform: Scale {
            origin.x: 0
            origin.y: 0
            xScale: root.width
            yScale: root.height
        }

        ShapePath {
            strokeColor: Theme.accentCyan
            strokeWidth: 0.006
            fillColor: "transparent"

            PathSvg {
                path: EncoderDrive.roadEdgePath
            }
        }
    }

    Item {
        width: Theme.encoderVehicleWidth
        height: Theme.encoderVehicleHeight

        anchors {
            bottom: parent.bottom
            bottomMargin: Theme.encoderVehicleBottomMargin
            horizontalCenter: parent.horizontalCenter
        }

        HypercarView {
            lateralOffset: EncoderDrive.vehicleLateralOffset
                           * Theme.encoderVehicleTravel
            yawDegrees: EncoderDrive.vehicleYawDegrees
            scale: Theme.encoderVehicleScale
        }
    }
}

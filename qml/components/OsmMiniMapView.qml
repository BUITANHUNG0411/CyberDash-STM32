import QtQuick
import QtQuick.Shapes
import QtLocation
import QtPositioning
import com.showcase

Item {
    id: root

    Plugin {
        id: osmPlugin

        name: "osm"

        PluginParameter {
            name: "osm.useragent"
            value: "QtStmAutomotiveSimulator/1.0"
        }

        PluginParameter {
            name: "osm.mapping.prefetching_style"
            value: "NoPrefetching"
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.mapFallback
        radius: Theme.radiusMd
        border.color: Theme.mapFrame
        border.width: 1
        clip: true

        Map {
            id: map

            anchors.fill: parent
            plugin: osmPlugin
            center: MapModel.viewportCenter
            zoomLevel: MapModel.zoomLevel
            bearing: 0
            tilt: 0
            color: Theme.mapFallback
            copyrightsVisible: true

            MapQuickItem {
                id: positionMarker

                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height / 2
                coordinate: MapModel.position

                sourceItem: Item {
                    width: 22
                    height: 28
                    rotation: MapModel.bearingDegrees
                    transformOrigin: Item.Center

                    Shape {
                        anchors.fill: parent

                        ShapePath {
                            fillColor: Theme.mapMarker
                            strokeColor: Theme.mapMarkerOutline
                            strokeWidth: 1.5

                            PathSvg {
                                path: "M 11 1 L 21 26 L 11 20 L 1 26 Z"
                            }
                        }
                    }
                }
            }

            DragHandler {
                target: null
                onTranslationChanged: MapModel.panByPixels(delta.x, delta.y, map.width, map.height)
            }

            WheelHandler {
                onWheel: MapModel.zoomByWheelDelta(event.angleDelta.y)
            }

            PinchHandler {
                onScaleChanged: MapModel.zoomByPinchScale(delta)
            }
        }

        Rectangle {
            width: followLabel.implicitWidth + Theme.spaceLg * 2
            height: followLabel.implicitHeight + Theme.spaceSm * 2
            radius: Theme.radiusPill
            color: Theme.mapStatusFill
            border.color: Theme.mapStatusBorder
            border.width: 1

            anchors {
                top: parent.top
                left: parent.left
                margins: Theme.spaceMd
            }

            Text {
                id: followLabel

                anchors.centerIn: parent
                color: Theme.textPrimary
                font.family: Theme.fontDisplay
                font.pixelSize: Theme.textXs
                font.letterSpacing: 1.2
                text: MapModel.followLabel
            }
        }

        Text {
            anchors.centerIn: parent
            color: Theme.textSecondary
            font.family: Theme.fontMain
            font.pixelSize: Theme.textXs
            text: map.errorString
            visible: map.error !== Map.NoError
        }
    }
}

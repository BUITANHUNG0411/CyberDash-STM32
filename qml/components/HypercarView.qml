import QtQuick
import QtQuick.Shapes
import com.showcase

Item {
    id: root

    property real lateralOffset: 0.0
    property real yawDegrees: 0.0

    implicitWidth: Theme.encoderVehicleWidth
    implicitHeight: Theme.encoderVehicleHeight
    x: root.lateralOffset
    rotation: root.yawDegrees
    transformOrigin: Item.Bottom

    Rectangle {
        x: 25
        y: 198
        width: 210
        height: 26
        radius: 13
        color: Theme.hypercarShadow
    }

    Shape {
        ShapePath {
            strokeWidth: 0
            fillColor: Theme.hypercarShadow

            PathSvg {
                path: "M 28 167 Q 40 161 51 176 L 47 204 Q 31 201 27 187 Z"
            }
        }

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.hypercarShadow

            PathSvg {
                path: "M 232 167 Q 220 161 209 176 L 213 204 Q 229 201 233 187 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 2
            fillColor: Theme.hypercarBody

            PathSvg {
                path: "M 29 170 Q 31 141 54 122 L 74 76 Q 88 43 130 40 Q 172 43 186 76 L 206 122 Q 229 141 231 170 L 224 197 Q 216 210 194 211 H 66 Q 44 210 36 197 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 1
            fillColor: Theme.hypercarGlass

            PathSvg {
                path: "M 39 169 Q 80 182 130 182 Q 180 182 221 169 L 217 193 Q 178 204 130 204 Q 82 204 43 193 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 2
            fillColor: Theme.hypercarGlass

            PathSvg {
                path: "M 74 121 L 90 77 Q 101 56 130 54 Q 159 56 170 77 L 186 121 Q 158 132 130 133 Q 102 132 74 121 Z"
            }
        }

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.hypercarShadow

            PathSvg {
                path: "M 94 78 Q 108 63 130 62 Q 152 63 166 78 L 175 111 Q 151 117 130 117 Q 109 117 85 111 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 1
            fillColor: Theme.hypercarGlass

            PathSvg {
                path: "M 55 116 L 205 116 L 213 124 L 203 130 L 57 130 L 47 124 Z"
            }
        }

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.hypercarBody

            PathSvg {
                path: "M 63 128 L 70 111 L 76 113 L 72 132 Z M 197 128 L 190 111 L 184 113 L 188 132 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 2
            fillColor: Theme.hypercarTailLamp

            PathSvg {
                path: "M 51 145 Q 75 138 101 143 L 89 158 Q 64 160 45 157 Z"
            }
        }

        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 2
            fillColor: Theme.hypercarTailLamp

            PathSvg {
                path: "M 209 145 Q 185 138 159 143 L 171 158 Q 196 160 215 157 Z"
            }
        }
    }

    Shape {
        ShapePath {
            strokeColor: Theme.hypercarHighlight
            strokeWidth: 2
            fillColor: "transparent"

            PathSvg {
                path: "M 40 169 Q 82 181 130 181 Q 178 181 220 169"
            }
        }

        ShapePath {
            strokeWidth: 0
            fillColor: Theme.hypercarHighlight

            PathSvg {
                path: "M 111 172 H 149 Q 152 172 152 175 Q 152 178 149 178 H 111 Q 108 178 108 175 Q 108 172 111 172 Z"
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

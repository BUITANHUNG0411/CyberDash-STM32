pragma Singleton
import QtQuick

QtObject {
    id: root

    // Cyberpunk Color Palette — Night (default) / Day (Light Glassmorphism)
    // Themed tokens are non-readonly because a Behavior cannot attach to a readonly property.
    property color backgroundDeepSpace: ThemeController.isNight ? "#0B0C10" : "#DDE7EE"
    // Accent theo drive mode (ECO/NORMAL/SPORT) × day/night — 6 biến thể.
    // SPORT dùng cam (không đỏ) để phân biệt rõ với warningRed/redline.
    property color accentCyan: DriveMode.driveMode === "eco"
                               ? (ThemeController.isNight ? "#66FC8F" : "#1E8A4C")
                               : DriveMode.driveMode === "sport"
                                 ? (ThemeController.isNight ? "#FF7A00" : "#C25600")
                                 : (ThemeController.isNight ? "#66FCF1" : "#00857C")
    readonly property color warningRed: "#FF3B30"
    property color parkingCaution: ThemeController.isNight ? "#FFB000" : "#A66800"
    property color parkingUnavailable: ThemeController.isNight ? "#74808C" : "#66727D"
    property color textPrimary: ThemeController.isNight ? "#FFFFFF" : "#1A2530"
    property color textSecondary: ThemeController.isNight ? "#C5C6C7" : "#4A5A68"
    property color textOnAccent: ThemeController.isNight ? "#0B0C10" : "#F2F7FA"

    property color glassPanelBase: ThemeController.isNight ? "#40151D26" : "#8CE8F0F8"
    property color glassPanelBorder: ThemeController.isNight ? "#802A3B4C" : "#80FFFFFF"
    property color trackInactive: ThemeController.isNight ? "#40FFFFFF" : "#401A2530"

    property color tickLitMajor: ThemeController.isNight ? "#FFB3CC" : "#D81B60"
    property color tickLitMinor: ThemeController.isNight ? "#FFFFFF" : "#0E8F88"
    property color tickDimMajor: ThemeController.isNight ? "#2A3B4C" : "#B8C6D1"
    property color tickDimMinor: ThemeController.isNight ? "#151D26" : "#D3DDE5"

    readonly property color coverFallback1: "#FF0055"
    readonly property color coverFallback2: "#4A00E0"
    readonly property color glassEdge: Qt.rgba(1.0, 1.0, 1.0, 0.15)
    // Physical bezel outline — identical in both themes
    readonly property color bezelStroke: "#80C5C6C7"

    // Smooth cross-fade when the theme flips (single place for the whole app)
    Behavior on backgroundDeepSpace { ColorAnimation { duration: durationTheme } }
    Behavior on accentCyan { ColorAnimation { duration: durationTheme } }
    Behavior on parkingCaution { ColorAnimation { duration: durationTheme } }
    Behavior on parkingUnavailable { ColorAnimation { duration: durationTheme } }
    Behavior on textPrimary { ColorAnimation { duration: durationTheme } }
    Behavior on textSecondary { ColorAnimation { duration: durationTheme } }
    Behavior on textOnAccent { ColorAnimation { duration: durationTheme } }
    Behavior on glassPanelBase { ColorAnimation { duration: durationTheme } }
    Behavior on glassPanelBorder { ColorAnimation { duration: durationTheme } }
    Behavior on trackInactive { ColorAnimation { duration: durationTheme } }
    Behavior on tickLitMajor { ColorAnimation { duration: durationTheme } }
    Behavior on tickLitMinor { ColorAnimation { duration: durationTheme } }
    Behavior on tickDimMajor { ColorAnimation { duration: durationTheme } }
    Behavior on tickDimMinor { ColorAnimation { duration: durationTheme } }

    // Animation Timings
    readonly property int durationTheme: 600
    readonly property int durationFast: 150
    readonly property int durationNormal: 300
    readonly property int durationSlow: 600
    readonly property int durationSpin: 900
    readonly property int durationCover: 8000
    readonly property int durationPress: 100
    readonly property int durationTick: 150
    readonly property int durationGauge: 250

    // Radii
    readonly property int radiusSm: 8
    readonly property int radiusMd: 16
    readonly property int radiusLg: 22
    readonly property int radiusPill: 32

    // Typography setup
    readonly property string fontMain: "Inter, Roboto, sans-serif"
    readonly property string fontDisplay: "Orbitron, Roboto, sans-serif"
    readonly property int textXs: 12
    readonly property int textSm: 16
    readonly property int textMd: 18
    readonly property int textLg: 20
    readonly property int textXl: 22
    readonly property int displayMd: 90
    readonly property int displayLg: 100

    // Spacing
    readonly property int spaceXs: 4
    readonly property int spaceSm: 6
    readonly property int spaceMd: 8
    readonly property int spaceLg: 10
    readonly property int spaceXl: 24
    readonly property int spaceXXl: 40

    // Bezel Geometry
    readonly property real bezelMargin: 20
    readonly property real dashboardMargin: 10
    readonly property real panelLift: 30
    // The arch shoulders move out while their outermost edge remains within the
    // framed window. Keep all bezel and dashboard spacing decisions together.
    readonly property real bezelArchLeftX: 190
    readonly property real bezelArchRightX: 970
    readonly property real bezelArchRadiusX: 170
    readonly property real bezelArchRadiusY: 380
    readonly property real bezelReflectionInset: 4
    readonly property real gaugeInsetLeft: 220
    readonly property real gaugeInsetRight: 920
    readonly property real centerPanelGap: 20
}

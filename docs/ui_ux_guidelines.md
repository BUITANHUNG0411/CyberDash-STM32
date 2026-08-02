# 🎨 UI/UX and QML Guidelines

> **AI Context**: Active design-system, layout-state, and animation rules for the Neon Cyberpunk automotive dashboard.

## 1. Canonical Visual Reference

The screenshot currently present in the repository is the canonical visual reference:

![Canonical dashboard reference](../resources/media/dashboard-preview.png)

Preserve its double-arch silhouette, dense but glanceable telemetry hierarchy, neon tick lighting, glass surfaces, and dark automotive character. If a later approved visual replaces it, update this guide and the root routing document together.

## 2. Theme and Accent System

> [!IMPORTANT]
> Bind reusable colors, spacing, geometry, radii, typography, and durations to `qml/Theme.qml`. Avoid new hard-coded visual constants in components.

The theme has day and night variants. Core background/text/glass/tick tokens switch through `ThemeController.isNight` and cross-fade through centralized `ColorAnimation` behaviors.

`Theme.accentCyan` is the stable cyan/teal accent in both day and night variants. `Theme.warningRed`
remains reserved for warnings and redlines.

## 3. Car Layout

`DashboardScreen` has one stable Car layout: a 160 km/h speed gauge, RPM right gauge, bottom
telemetry bar, and CenterHub. There is no vehicle-mode state machine, form-factor morphing, or
dynamic layout Loader.

## 4. CenterHub

Car mode uses `CenterHub.qml` as the persistent home of `MusicPlayer` and `ParkingAssistView`. The player owns no navigation state; its library, playback, and scrubber
remain C++-backed through `MusicViewModel`.

While reverse is active, the C++-owned `CenterHubViewModel` keeps `MusicPlayer` visible for clear
and caution samples, then selects `ParkingAssistView` automatically when a live sample is below
30 cm; users may also drag horizontally between Music and Distance Warning. The `StackLayout`
keeps CenterHub pages alive, while C++ owns the 80 px gesture threshold and a live critical
sample overrides manual Music requests. Parking Assist is a rectangular
dark-glass OEM panel. Its hierarchy is a compact two-row header with a left-aligned `REAR PARK
ASSIST` title and a right-aligned ultrasonic health label, followed by a large centred distance
readout, a status label, an abstract centre obstacle block above a bumper line, and an eight-segment
proximity track at the bottom. The obstacle block moves
only along the centre axis from `ParkingAssist.proximityProgress`; a single rear sensor does not
claim left/right obstacle position. `ParkingAssist.proximitySegments` lights the track without
QML-side calculations. The health label distinguishes live, stale, and unavailable input; cyan
represents clear/live, amber caution or stale, warning-red stop, and the unavailable Theme token
invalid input. Distance and obstacle position may animate numerically; text strings themselves do
not animate. Only the STOP bumper pulse repeats. This is a sensor UI, not a camera image. Leaving
the critical range returns to the requested non-critical CenterHub page.

The compact original Double Arch geometry is centralized in `Theme.qml`. The bezel path endpoints,
two gauge centres, and CenterHub overlap margin remain fixed for the Car layout.

Do not put window-drag handlers over the player controls, `PathView`, or the scrubber. The
CenterHub drag handler is a separate null-target handler that only commits a horizontal page
change after the gesture threshold; ordinary clicks remain available to child controls.

## 5. Animation Rules

Animate numeric visual properties, not strings. For example:

```qml
Item {
    property real displayedValue: vm.speed

    Behavior on displayedValue {
        NumberAnimation {
            duration: Theme.durationGauge
            easing.type: Easing.OutQuad
        }
    }

    Rectangle {
        rotation: parent.displayedValue
    }

    Text {
        text: vm.displaySpeed
    }
}
```

Never apply `NumberAnimation` to `Text.text`. Keep formatting in C++ when it needs logic or fixed precision.

## 8. MultiEffect Safety

> [!WARNING]
> A `MultiEffect` source must not contain that effect. Never capture an ancestor such as `source: parent` when it creates recursive capture and frozen accumulated frames. Prefer a non-recursive sibling source.

Hide a sibling source when it exists solely as an input to the effect, as with the music backdrop and some icon/text sources. A visible sibling is correct when the original must also render; gauge ticks are valid visible sources for their bloom effects.

## 9. Zero JavaScript

QML may use bindings, ternaries, declarative states, and a single direct call to a C++ invokable. Interaction state and calculations belong in C++. The music scrubber is the reference implementation: QML forwards pointer coordinates, while `MusicPlayerViewModel` owns drag state, clamping, normalization, and seek requests.

## Troubleshooting

- **Theme change recolors only part of the UI:** replace local colors with `Theme` tokens and confirm the token has a centralized `Behavior`.
- **Blur or glow freezes:** inspect `MultiEffect.source` for recursive parent capture and replace it with a sibling source.
- **Text animation warns or jumps:** animate a numeric backing property and bind `Text.text` to its display value.
- **Swipe or scrub drags the window:** restrict the window `DragHandler` to the top drag strip.
- **Center panel touches a gauge:** adjust the centralized baseline bezel/gauge geometry and
  `Theme.centerPanelGap` together; do not add ad-hoc margins in a component.

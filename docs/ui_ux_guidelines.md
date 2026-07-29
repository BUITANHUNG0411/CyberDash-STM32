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

Drive mode selects the active accent:

| Drive mode | Night accent | Day accent | Purpose |
|---|---|---|---|
| ECO | `#66FC8F` | `#1E8A4C` | Green efficiency state |
| NORMAL | `#66FCF1` | `#00857C` | Cyan/teal default state |
| SPORT | `#FF7A00` | `#C25600` | Orange performance state |

`Theme.warningRed` remains reserved for warnings and redlines; SPORT must not reuse it.

## 3. Vehicle Layout States

`DashboardScreen.state` binds to the C++ `VehicleMode.vehicleMode` property and supports exactly:

- **Car:** 160 km/h speed gauge, RPM right gauge, bottom bar, and CenterHub.
- **Bike:** 60 km/h speed gauge, large battery presentation, center hub and bottom bar hidden.
- **Scooter:** 120 km/h speed gauge, battery right gauge, and `RangeTripCard` center content.

Use declarative `State`, `PropertyChanges`, and `Transition`. The Car state remains the base binding set so leaving Bike/Scooter restores original bindings.

## 4. Cockpit Context Rail

`CockpitContextRail` is a non-interactive, five-pill chrome rail anchored below the top bar. It is
hidden until boot stage 2 and remains visible in Car, Bike, and Scooter modes. It displays
C++-owned vehicle mode, drive mode, theme, telemetry source, and Safety Lab state. It uses only
Theme tokens and must not overlap gauges or the center panel, introduce QML state or calculations,
or alter Safety Lab availability.

## 5. CenterHub

Car mode uses `CenterHub.qml` as the persistent home of `MusicPlayer`. The player owns no navigation state; its library, playback, and scrubber remain C++-backed through `MusicViewModel`.

While reverse is active, the C++-owned `CenterHubViewModel` keeps `MusicPlayer` visible for clear
and caution samples, then selects `ParkingAssistView` automatically when a live sample is below
30 cm; users may also drag horizontally between the Music and Distance Warning tabs. The
`StackLayout` keeps both siblings alive, while C++ owns the 80 px gesture threshold and a live
critical sample overrides a manual Music request. Parking Assist is a rectangular
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
the critical range returns to Music automatically.

The compact original Double Arch geometry is centralized in `Theme.qml`. The bezel path endpoints
and the two gauge centres remain at their baseline positions; `DashboardScreen.centerPanel` keeps
the original tokenized overlap margin to preserve the established depth layering across vehicle
modes.

Do not put window-drag handlers over the player controls, `PathView`, or the scrubber. The
CenterHub drag handler is a separate null-target handler that only commits a horizontal page
change after the gesture threshold; ordinary clicks remain available to child controls.

## 6. Cyber Safety Mock Lab

The Car-only `SafetyScenarioOverlay` is an anchored sibling above `CenterHub` in the existing
center panel; it is not a third CenterHub page and does not change the root vehicle state or its
transitions. C++ owns the availability gate, deterministic timeline, acknowledgement window,
display text, and eight risk segments. QML only renders those properties and forwards the direct
start/acknowledge/exit invokables. While active, the CenterHub render fades out but its object
remains alive, so the lab is an unambiguous center-panel takeover rather than a layered duplicate.
Its compact two-zone header keeps the C++-supplied `SAFETY LAB` title and scenario instruction on
the left, with the direct `EXIT` action reserved on the right; do not centre a long title beneath
the exit action or reintroduce oversized letter spacing.

While visible, the overlay continuously shows `DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL`.
It presents only an abstract forward threat gate—never real sensing, a camera, map, ADAS feature,
or a claim of vehicle control. Severity reuses `Theme.accentCyan`, `Theme.parkingCaution`, and
`Theme.warningRed`; only numeric `x`, opacity, and scale may animate. Do not add a CenterHub page,
root state, local QML timeline, or local acknowledgement state.

## 7. Animation Rules

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

Vehicle morphing uses the dip transition: fade and scale both arches down, apply property swaps while invisible, then restore opacity and scale with an OutBack rise. Animate opacity and scale, not complex subtree width/height.

## 8. MultiEffect Safety

> [!WARNING]
> A `MultiEffect` source must not contain that effect. Never capture an ancestor such as `source: parent` when it creates recursive capture and frozen accumulated frames. Prefer a non-recursive sibling source.

Hide a sibling source when it exists solely as an input to the effect, as with the music backdrop and some icon/text sources. A visible sibling is correct when the original must also render; gauge ticks are valid visible sources for their bloom effects.

## 9. Zero JavaScript

QML may use bindings, ternaries, declarative states, and a single direct call to a C++ invokable. Interaction state and calculations belong in C++. The music scrubber is the reference implementation: QML forwards pointer coordinates, while `MusicPlayerViewModel` owns drag state, clamping, normalization, and seek requests.

## Troubleshooting

- **Theme change recolors only part of the UI:** replace local colors with `Theme` tokens and confirm the token has a centralized `Behavior`.
- **Morph shows tick-label popping:** keep the property swap inside the invisible midpoint of the dip transition.
- **Blur or glow freezes:** inspect `MultiEffect.source` for recursive parent capture and replace it with a sibling source.
- **Text animation warns or jumps:** animate a numeric backing property and bind `Text.text` to its display value.
- **Swipe or scrub drags the window:** restrict the window `DragHandler` to the top drag strip.
- **Center panel touches a gauge:** adjust the centralized baseline bezel/gauge geometry and
  `Theme.centerPanelGap` together; do not add ad-hoc margins in a component.

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

## 4. CenterHub

Car mode uses `CenterHub.qml` as the persistent home of `MusicPlayer`. The player owns no navigation state; its library, playback, and scrubber remain C++-backed through `MusicViewModel`.

While reverse is active, the C++-owned `CenterHubViewModel` selects `ParkingAssistView` and
keeps `MusicPlayer` alive as a static sibling. Parking Assist is a rectangular dark-glass OEM
panel. Its hierarchy is a left-aligned `REAR PARK ASSIST` header, a right-aligned ultrasonic
health label, a large centred distance readout, a status label, an abstract centre obstacle block
above a bumper line, and an eight-segment proximity track at the bottom. The obstacle block moves
only along the centre axis from `ParkingAssist.proximityProgress`; a single rear sensor does not
claim left/right obstacle position. `ParkingAssist.proximitySegments` lights the track without
QML-side calculations. Cyan represents clear, amber caution, warning-red stop, and the
unavailable Theme token stale/invalid input. Only the STOP bumper pulse repeats; position, opacity,
and colour may animate, never text. This is a sensor UI, not a camera image. Leaving reverse
returns to Music automatically.

The widened Double Arch geometry is also centralized in `Theme.qml`. The bezel path endpoints and
the two gauge centres are moved outward together; `DashboardScreen.centerPanel` uses a positive
`Theme.centerPanelGap` on both sides. Do not use negative panel margins to recover space, because
the CenterHub must remain visibly separated from both illuminated gauge rings across vehicle modes.

Do not put window-drag handlers over the player controls, `PathView`, or the scrubber.

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

Vehicle morphing uses the dip transition: fade and scale both arches down, apply property swaps while invisible, then restore opacity and scale with an OutBack rise. Animate opacity and scale, not complex subtree width/height.

## 6. MultiEffect Safety

> [!WARNING]
> A `MultiEffect` source must not contain that effect. Never capture an ancestor such as `source: parent` when it creates recursive capture and frozen accumulated frames. Prefer a non-recursive sibling source.

Hide a sibling source when it exists solely as an input to the effect, as with the music backdrop and some icon/text sources. A visible sibling is correct when the original must also render; gauge ticks are valid visible sources for their bloom effects.

## 7. Zero JavaScript

QML may use bindings, ternaries, declarative states, and a single direct call to a C++ invokable. Interaction state and calculations belong in C++. The music scrubber is the reference implementation: QML forwards pointer coordinates, while `MusicPlayerViewModel` owns drag state, clamping, normalization, and seek requests.

## Troubleshooting

- **Theme change recolors only part of the UI:** replace local colors with `Theme` tokens and confirm the token has a centralized `Behavior`.
- **Morph shows tick-label popping:** keep the property swap inside the invisible midpoint of the dip transition.
- **Blur or glow freezes:** inspect `MultiEffect.source` for recursive parent capture and replace it with a sibling source.
- **Text animation warns or jumps:** animate a numeric backing property and bind `Text.text` to its display value.
- **Swipe or scrub drags the window:** restrict the window `DragHandler` to the top drag strip.
- **Center panel touches a gauge:** adjust the centralized bezel/gauge geometry and positive
  `Theme.centerPanelGap` together; never compensate with overlap margins in a component.

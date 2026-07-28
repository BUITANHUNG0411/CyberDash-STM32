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

Car mode uses `CenterHub.qml`, a `SwipeView` containing static Music and OSM mini-map pages:

- `MusicPlayer` preserves its lifetime while the user views the map.
- `OsmMiniMapView` is a rectangular glass-panel map with a dark fallback, cyan frame, a compact marker, and a restrained `FOLLOW`/`EXPLORE` status pill using centralized `Theme` tokens.
- The map remains north-up; only the marker rotates from C++-provided bearing.
- User drag, wheel, and pinch gestures directly call `MapModel` invokables. C++ owns Web-Mercator pan/zoom math, follow/explore state, and the four-second idle return to follow mode.
- OSM attribution remains visible and must not be covered by the pill or page indicator. The OSM plugin uses the application User-Agent and `NoPrefetching`; do not add route lines, destination/search controls, routing, bulk downloads, or offline tile bundles.

`OsmMiniMapView` contains no QML-side map calculations, timers, or mutable interaction state. It binds declaratively to `MapModel.position`, `bearingDegrees`, `viewportCenter`, `zoomLevel`, and `followLabel`.

Do not put window-drag handlers over this interactive region; the top drag strip must not steal gestures from `SwipeView`, `PathView`, or the scrubber.

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
- **Map rotates with the marker:** keep `Map.bearing` at `0`; bind only the marker rotation to `MapModel.bearingDegrees`.
- **Map stays in explore mode:** confirm all gesture updates restart the C++ follow timeout, then check that follow mode recenters and restores default zoom after four idle seconds.
- **OSM attribution is hidden:** move the map status pill or page indicator; attribution must remain visible.

# Center Hub Manual Swipe Design

## Goal

Allow the center panel to be manually dragged between two tabs: Music Player and
Distance Warning, while preserving automatic safety selection from live ultrasonic
telemetry.

## Design

`CenterHub.qml` keeps its `StackLayout` so both pages remain instantiated and the
music player keeps its playback state. A parent-level `DragHandler` with a null
target captures horizontal drags after nested controls fail to claim a click. QML
only forwards pointer state and translation to C++.

`CenterHubViewModel` owns the gesture state and an 80 px horizontal commit threshold.
On release, a left drag selects Distance Warning and a right drag selects Music.
Manual requests are validated through the existing two-page enum. When
`ParkingAssistViewModel::criticalProximity()` is true, Music cannot override the
automatic Parking Warning selection; this keeps a live `<30 cm` warning visible.

The warning page is not hidden by its own `visible` binding. `StackLayout` controls
which page is shown, so a user can inspect clear, caution, stale, or unavailable
distance data manually. Automatic selection remains STOP-gated and unchanged.

## Constraints

- Zero JavaScript in QML; all threshold and state logic lives in C++.
- QML remains independent of SimulatorService versus future STM32 adapters.
- Existing `<30 cm` auto-switch and mock data seam remain intact.
- No window-drag handler is placed over the center panel.

## Verification

Add focused QtTest coverage for left/right manual drags, short drags, invalid pages,
and critical-distance safety override. Then run the full configure/build/CTest,
Zero-JS, QML lint, module qmllint, and offscreen smoke checks.

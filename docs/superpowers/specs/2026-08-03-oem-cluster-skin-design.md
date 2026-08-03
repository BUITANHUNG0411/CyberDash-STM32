# OEM Instrument Cluster Skin Design

**Date:** 2026-08-03
**Status:** Implemented locally, pending human review
**Scope:** Shared CenterHub panel chrome and page-specific instrument accents

## Goal

Translate the physical dashboard reference into a more convincing OEM instrument
cluster UI while preserving the existing single Car dashboard, double-arch geometry,
CenterHub pages, and C++ ownership boundaries.

## Design Direction

The current dashboard has the right silhouette but its CenterHub surfaces still read
as independent glass cards. The new skin adds a shared physical frame language:

- `InstrumentFrame.qml` renders a transparent outer rim, inner glass lip, and a short
  accent highlight without owning any interaction or business state.
- `GlassPanel.qml` exposes `accentColor` and renders `InstrumentFrame` above its
  content, so Trip and Parking inherit the same cluster hardware treatment.
- `MusicPlayer.qml` receives the same frame overlay because it intentionally does not
  use `GlassPanel` for its cover-art backdrop.
- Parking binds the frame accent to its existing C++-derived proximity color; Trip and
  Music retain the cyan cluster accent.
- `CenterHub.qml` adds three compact status LEDs above the existing Music/Park/Trip
  tabs. The active Park LED becomes amber or warning-red using existing parking state;
  no new page or vehicle-mode state machine is introduced.
- The cluster chrome is intentionally legible at the current wide dashboard scale:
  the frame line is 2 px, the accent highlight spans 48% of the panel width, and
  status LEDs are 32×6 px with 18 px spacing.

## Constraints

- QML remains passive and contains zero executable JavaScript.
- Existing `CenterHubViewModel`, `ParkingAssistViewModel`, `TripComputerViewModel`,
  and `MusicPlayerViewModel` contracts remain unchanged.
- The double-arch geometry and stable `StackLayout` remain unchanged.
- Use centralized `Theme.qml` tokens for frame geometry, LED dimensions, opacity, and
  timing; do not add visual constants directly to components.
- Do not fake left/right parking position: the single ultrasonic sensor remains a
  centre-axis presentation.
- Do not add dependencies, delete files, or commit changes.

## Acceptance Criteria

- All CenterHub page surfaces show the shared OEM frame treatment.
- Parking frame and status LED reflect the existing clear/caution/stop color state.
- Trip and Music retain cyan accents and remain visually distinct from Parking.
- The three LEDs are declarative bindings to the existing C++ page/proximity state.
- A source-contract test protects registration, frame usage, accent wiring, and the
  zero-JavaScript boundary.
- Configure, build, all CTest targets, zero-JS scan, QML review, and offscreen smoke
  verification complete without new errors.

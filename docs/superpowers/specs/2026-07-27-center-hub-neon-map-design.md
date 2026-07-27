# Design Spec: Center Hub (Music ⇄ Map) + Neon Map

> **Date**: 2026-07-27
> **Status**: Approved by user (brainstorming session)
> **Phase**: 16 (next after Drive Modes + Trip Computer)

## 1. Goal

Turn the fixed center panel (currently MusicPlayer only) into a **swipeable hub** with
two pages — the existing Music Player and a new stylized **Neon Map** whose position
marker advances along a route using REAL travelled distance from the trip computer.

Scope decisions (user-approved):
- Pages: **Music + Map** only (more pages can be appended later).
- Map style: **stylized neon map** drawn with QtQuick.Shapes — no Qt Location,
  no network tiles, no new heavy dependencies.
- The hub exists **only in Car mode**. Scooter keeps RangeTripCard, Bike keeps the
  empty center — the Phase-14 state machine changes only its target
  (`musicPlayer.opacity` → `centerHub.opacity`).

Global directives apply: Zero-JS QML, MVVM (new behavior in C++), Simulator ↔ Serial
swap invariance, `-Wconversion`-clean C++17.

## 2. Architecture

### 2.1 Container: `SwipeView` (Approach A — chosen)

`qml/components/CenterHub.qml`:

- `SwipeView` (from `QtQuick.Controls.Basic` — first Controls usage in the repo,
  justified because SwipeView's statically declared children are instantiated once
  and NEVER destroyed, preserving the "MusicPlayer must stay alive" decision from
  Phase 14) containing two static children: `MusicPlayer` and `NeonMapView`.
- `PageIndicator` (top-center, dots tinted `Theme.accentCyan` / `Theme.trackInactive`);
  this matches the approved implementation and keeps the dots clear of the
  bottom playback controls.
  bound to the SwipeView's `count`/`currentIndex`.
- Swipe gesture is native; `currentIndex` is view-local state (same precedent as the
  MusicPlayer PathView's `currentIndex`). No new ViewModel for the page index (YAGNI).

Rejected: horizontal `ListView` + snap (model-delegates risk destroy/recreate of
MusicPlayer), `StackLayout` + buttons (no swipe gesture).

### 2.2 New C++: `MapViewModel` (context property `MapModel`)

One-ViewModel-per-Concern. API:

| Member | Kind | Behavior |
|---|---|---|
| `routeProgress` | `Q_PROPERTY(qreal, READ + NOTIFY routeProgressChanged)` | 0.0 → 1.0 position along the route loop; wraps (fmod). Default 0.0. |
| `MapViewModel(double routeLengthKm = 2.0, QObject *parent = nullptr)` | ctor | Route length injectable for tests. |
| `updateDistance(double odometerKm)` | public slot | `routeProgress = fmod(odometerKm / routeLengthKm, 1.0)`; emits only on change. |

Wiring in `main.cpp` (after `tripVm` exists):
`QObject::connect(&tripVm, &TripComputerViewModel::tripChanged, [&]{ mapVm.updateDistance(tripVm.odometerKm()); });`
The marker therefore moves with genuine telemetry-integrated distance — it stops when
the vehicle stops and speeds up when the vehicle speeds up, for Simulator AND Serial.

### 2.3 `qml/components/NeonMapView.qml` (pure view)

- Dark glass base (reuse `GlassPanel` styling tokens), subtle street grid
  (`Repeater` of thin `Rectangle` lines at low opacity).
- A few background "streets": one `Shape` with grey `ShapePath` strokes.
- **Main route**: a closed-loop `PathSvg` stroked in `Theme.accentCyan` (inherits
  ECO/NORMAL/SPORT accent for free) + neon bloom via `MultiEffect` whose source is a
  **hidden sibling** item (per the MultiEffect Sibling Source rule).
- **Position marker**: small triangle/arrow `Shape`; its `x`/`y`/`rotation` come from
  `PathInterpolator { path: <same route path>; progress: MapModel.routeProgress }` —
  the arrow auto-orients to the road direction via the interpolator's `angle`.
- Top corner: `"TRIP " + TripComputer.tripDisplay` label (reuses existing formatted
  string — no new formatting code).

## 3. DashboardScreen integration

- `centerPanel` hosts `CenterHub { id: centerHub }` in place of the direct
  `MusicPlayer`.
- Phase-14 states: scooter's `PropertyChanges { target: musicPlayer; opacity: 0 }`
  becomes `PropertyChanges { target: centerHub; opacity: 0 }` (same for bike);
  transition Phase-C `NumberAnimation` targets list swaps `musicPlayer` → `centerHub`.
  `scooterCard` Loader and bike layout are untouched.
- Boot choreography untouched: `centerPanel.opacity` keeps the `bootStage` binding;
  the hub is a child.

## 4. Edge cases

- **Swipe during morph transition**: hub fades as a whole (opacity on `centerHub`);
  SwipeView state (current page) is preserved because children are never destroyed.
- **Route wrap**: `fmod` keeps `routeProgress` in [0,1); `PathInterpolator` handles
  0↔1 continuity on a closed path.
- **No telemetry / vehicle stopped**: `tripChanged` stops firing → progress freezes —
  correct behavior, no special handling.
- **Music keeps playing while on the Map page**: audio lives in
  `MusicPlayerViewModel` (C++); the hidden page keeps rendering nothing
  (SwipeView clips non-current pages) but state is intact.

## 5. Testing plan (TDD — tests first)

In `tests/main.cpp`:
1. `testMapDefaults` — fresh VM: `routeProgress == 0.0`.
2. `testMapProgressAdvances` — `MapViewModel map(2.0); map.updateDistance(0.5);` →
   `routeProgress == 0.25`, NOTIFY fired once; same value again → no second emit.
3. `testMapProgressWraps` — `map.updateDistance(2.5)` → `0.25` (wraps past 1.0).
4. Existing suite stays green; 8 s smoke run clean; screenshots of both hub pages.

## 6. File impact

| File | Change |
|---|---|
| `src/viewmodels/MapViewModel.{h,cpp}` | **New** — route progress from odometer. |
| `qml/components/CenterHub.qml` | **New** — SwipeView + PageIndicator. |
| `qml/components/NeonMapView.qml` | **New** — stylized neon map + marker. |
| `qml/screens/DashboardScreen.qml` | Center panel hosts CenterHub; state/transition targets renamed. |
| `src/main.cpp` | Register `MapModel`, connect trip → map. |
| `CMakeLists.txt` / `tests/CMakeLists.txt` / `tests/main.cpp` | Sources + tests. |
| `docs/tasks_board.md` / `docs/journal.md` / `README.md` | Progress + decisions + feature bullet. |

## 7. Out of scope (YAGNI)

- Real map tiles / Qt Location / GPS coordinates.
- Persisting the selected hub page.
- Additional hub pages (Trip page, settings) — append later.
- Turn-by-turn navigation UI.

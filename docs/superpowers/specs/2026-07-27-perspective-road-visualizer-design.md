# Design Spec: Perspective Single-Road Visualizer

> **Status:** Superseded by the encoder-driven arrow-road spec and plan set in July 2026.
> This document is retained as historical context for the earlier perspective-road implementation.

> **AI Context**: Approved Phase 18 design for replacing the route-loop map with a mock-driven, hardware-ready, single-road perspective visualizer.
> **Date**: 2026-07-27
> **Status**: Approved by user during brainstorming; implementation not started
> **Phase**: 18

## 1. Understanding Summary

- Replace the current general neon-map presentation with one continuous road viewed in pseudo-3D perspective.
- Keep the vehicle marker fixed near the bottom center while the road scrolls toward the viewer.
- Render one lane only, with two neon edges and a dashed center line; do not show a city map, intersections, road names, or journey history.
- Run an automatic mock scenario first: straight, left curve, straight, right curve, then repeat with smooth transitions.
- Model turns from left/right wheel motion so the same view can later consume physical encoder telemetry.
- Keep all motion, curvature, geometry, recycling, clamping, and scenario state in C++17. QML remains a passive Zero-JavaScript view.
- Preserve the existing CenterHub swipe behavior and MusicPlayer lifetime.

This feature is a road-motion visualizer, not a navigation system or a globally localized map.

## 2. Assumptions and Non-Functional Requirements

### 2.1 Approved assumptions

- The first implementation uses deterministic mock wheel speeds; it does not read physical encoders.
- Positive forward wheel speed is the only required motion mode. Reverse motion and in-place rotation are out of scope.
- The road bends toward the commanded vehicle turn: right wheel faster than left produces a left bend; left wheel faster than right produces a right bend.
- The vehicle marker remains fixed. Apparent travel comes from segment depth and center-line movement.
- Mock telemetry continues independently of which CenterHub page is visible. Returning to the road page shows the current scenario state rather than restarting it.
- The existing `NeonMapView`/route-loop concept and `MapViewModel::routeProgress` are superseded rather than maintained as a second map model.

### 2.2 Performance and scale

- Target a stable 30 Hz C++ update cadence with a 33 ms timer interval. The design must not depend on display refresh rate.
- Use a fixed model size of 24 road segments unless profiling justifies a small adjustment.
- Reuse segment rows in place. Do not allocate or destroy QObjects on every tick.
- Each tick performs bounded O(segment-count) arithmetic on the GUI thread and no blocking I/O.
- QML uses no imperative animation loop. It renders normalized geometry supplied by C++.
- Glow must use a non-recursive sibling source and must be disabled when the road page is not visible if the final component uses `MultiEffect`.

### 2.3 Reliability, security, and maintenance

- Non-finite wheel speeds, non-positive or excessive elapsed time, and non-finite intermediate geometry must never reach QML.
- A stale input policy stops apparent road movement; it must not keep extrapolating indefinitely.
- The feature processes local telemetry only and introduces no network, location, user identity, or privacy surface.
- Ownership follows Qt parent ownership. Timers live with their owning service or ViewModel and run on the GUI thread.
- Mock and future serial sources must feed the same C++ wheel-telemetry boundary. QML must remain unchanged when the source changes.

## 3. Considered Approaches

### 3.1 Chosen: reusable road-segment model driven by C++

A fixed-size `QAbstractListModel` exposes normalized trapezoid geometry for road slices and dashed center marks. C++ advances, curves, clamps, and recycles rows; QML delegates only bind role values to visual primitives.

This approach is deterministic, testable without a display, compatible with Zero-JavaScript QML, and directly adaptable to real encoder telemetry.

### 3.2 Rejected: one continuously rebuilt `ShapePath`

A single path can produce smooth outlines, but moving dashed markings and perspective widths require additional state and frequent path reconstruction. It produces a less explicit hardware-to-visual contract and is harder to verify row by row.

### 3.3 Rejected: `ShaderEffect`

A fragment shader could render a polished road efficiently, but it moves important curvature behavior into shader code, weakens deterministic C++ testing, and increases maintenance complexity before hardware input exists.

## 4. Architecture

```text
MockWheelTelemetryService
  -> wheelTelemetryUpdated(leftSpeed, rightSpeed, elapsedMs)
  -> RoadMotionViewModel
  -> fixed RoadSegmentModel roles + motion properties
  -> PerspectiveRoadView.qml
```

The future hardware path reuses the same downstream boundary:

```text
STM32 encoder counters
  -> SerialTelemetryParser / SerialService extension
  -> wheelTelemetryUpdated(leftSpeed, rightSpeed, elapsedMs)
  -> RoadMotionViewModel
  -> unchanged PerspectiveRoadView.qml
```

No common QObject base class is required for the mock and serial producers. `main.cpp` owns source selection and connects either producer to the same typed `RoadMotionViewModel` update method, following the existing Simulator/Serial convergence pattern.

## 5. Components and Contracts

### 5.1 Wheel telemetry value

Introduce a small C++ value representing one processed motion sample:

| Field | Type | Unit | Contract |
|---|---|---|---|
| `leftWheelSpeed` | `double` | normalized forward units/second | Finite and nonnegative |
| `rightWheelSpeed` | `double` | normalized forward units/second | Finite and nonnegative |
| `elapsedMs` | `qint64` | milliseconds | Positive and bounded |

The initial mock values are normalized rather than physical metres per second. The later encoder adapter will convert tick deltas using wheel circumference, ticks per revolution, and axle track before producing the same logical sample.

PWM is explicitly not a position or turn sensor. Future direction estimation must come from encoder deltas; PWM may remain a command or diagnostic value only.

### 5.2 `MockWheelTelemetryService`

`MockWheelTelemetryService` owns a parented `QTimer` and emits deterministic wheel-speed samples. Its scenario is a continuous loop:

| Stage | Left wheel | Right wheel | Visual result |
|---|---:|---:|---|
| Straight A | Equal | Equal | Centered straight road |
| Left curve | Lower | Higher | Road ahead bends left |
| Straight B | Equal | Equal | Curvature returns smoothly to zero |
| Right curve | Higher | Lower | Road ahead bends right |

The default loop contains four 4-second stages with normalized target wheel
speeds: straight `(1.0, 1.0)`, left curve `(0.65, 1.0)`, straight
`(1.0, 1.0)`, and right curve `(1.0, 0.65)`. The first 1 second of each stage
smoothly interpolates from the previous target. Stage duration, transition
duration, and wheel targets are injectable configuration values so tests can
advance a short deterministic scenario. The service must not emit abrupt
curvature steps.

The service is responsible only for scenario timing and wheel samples. It does not create road geometry and has no QML API.

### 5.3 `RoadMotionViewModel`

`RoadMotionViewModel` is a `QAbstractListModel` and the single C++ home for road state. It owns:

- wheel-sample validation;
- forward-speed and curvature derivation;
- smoothing between target and displayed curvature;
- segment depth advancement;
- perspective center/width calculation;
- dashed-line phase;
- segment recycling;
- stale-sample behavior.

The conceptual motion terms are:

```text
forwardSpeed = (leftWheelSpeed + rightWheelSpeed) / 2
turnRate     = (rightWheelSpeed - leftWheelSpeed) / wheelTrack
curvature    = turnRate / max(forwardSpeed, minimumForwardSpeed)
```

The initial normalized `wheelTrack` is `1.0`, `minimumForwardSpeed` is `0.05`,
and absolute curvature is clamped to `0.75`. These equations define direction,
not a claim of field-calibrated odometry. C++ treats forward speed below the
minimum as stopped. Physical wheel dimensions and encoder calibration replace
the normalized constants only in the future adapter.

The model exposes normalized roles so the view scales to the CenterHub without owning geometry logic:

| Role | Range | Meaning |
|---|---|---|
| `leftNearX`, `rightNearX` | bounded normalized X | Near edge of the segment |
| `leftFarX`, `rightFarX` | bounded normalized X | Far edge of the segment |
| `nearY`, `farY` | `[0, 1]` | Perspective depth endpoints |
| `centerNearX`, `centerFarX` | bounded normalized X | Dashed center-line endpoints |
| `centerLineVisible` | boolean | Alternating dashed-mark state |
| `segmentOpacity` | `[0, 1]` | Horizon fade |

`PerspectiveRoadView.qml` may multiply normalized coordinates by its width and height in declarative bindings. It must not derive curvature, update depth, branch on scenario stages, mutate segment state, or recycle rows.

### 5.4 Public ViewModel behavior

The planned C++ surface includes:

| Member | Kind | Purpose |
|---|---|---|
| `updateWheelMotion(double left, double right, qint64 elapsedMs)` | public slot | Validate and apply one source-independent motion sample |
| `resetRoad()` | public method | Restore deterministic straight-road geometry for tests/startup |
| `forwardSpeed` | read-only `Q_PROPERTY` | Effective finite mock/hardware speed |
| `curvature` | read-only `Q_PROPERTY` | Smoothed signed bend, clamped to the visual limit |
| segment roles | `QAbstractListModel` | Normalized geometry consumed by QML |

Effective-value signals emit only when their published value changes. Model updates use precise `dataChanged` ranges and roles rather than resetting the model every frame.

### 5.5 `PerspectiveRoadView.qml`

The component replaces the visual content of `NeonMapView.qml`. It contains:

- a dark glass/perspective background using existing `Theme` tokens;
- a subtle horizon glow;
- repeated trapezoid road slices from `RoadMotionViewModel`;
- two neon road edges;
- repeated dashed center marks;
- one fixed vehicle/arrow marker near the bottom center;
- an optional compact mock-status label if it can be expressed through a direct C++ display property without clutter.

The road narrows toward the horizon and widens toward the bottom. Near segments move faster in screen space because perspective geometry is supplied by C++, producing forward motion without moving the vehicle marker.

The component contains no JavaScript functions, block handlers, mutable local state, `Math.*`, or timer-driven calculations. Reusable visual constants belong in `Theme.qml`.

### 5.6 CenterHub and application wiring

- Keep `CenterHub.qml`, its two static pages, swipe gesture, and top-center page indicator.
- Replace the route-loop page with `PerspectiveRoadView`.
- Keep MusicPlayer alive while the road page is selected and keep road telemetry alive while MusicPlayer is selected.
- Expose the model through a focused context property such as `RoadMotion`.
- Construct the mock service and ViewModel in `main.cpp`, connect the mock signal to `updateWheelMotion`, and start the mock only after all connections exist.
- Remove obsolete trip-to-map wiring and remove `MapViewModel` only after replacement tests cover the new boundary.

Car mode continues to own CenterHub. Bike and Scooter layouts remain unchanged.

## 6. Motion, Perspective, and Turn Semantics

Each segment has a normalized depth. C++ advances depth using forward speed and elapsed time. Perspective projection maps depth to:

- vertical position;
- road half-width;
- horizon opacity;
- lateral displacement accumulated from curvature.

When a segment passes the near boundary, it is recycled to the far boundary while preserving model row identity. Its dash visibility follows a stable alternating sequence so the center line appears to move continuously.

Turn semantics are fixed:

- equal wheels: target curvature zero;
- right wheel faster: positive/left visual bend;
- left wheel faster: negative/right visual bend;
- stopped wheels: no depth movement;
- curvature returns gradually to zero during the next straight stage.

The visualizer shows only the current road corridor. It does not accumulate `(x, y, heading)` history and does not attempt global localization.

## 7. Error and Edge-Case Behavior

- Reject or neutralize non-finite wheel speeds before any calculation.
- Clamp negative wheel speeds to the supported stopped/forward domain for Phase 18.
- Ignore non-positive `elapsedMs`; clamp values above 100 ms to prevent jumps
  after suspension or debugger pauses.
- Avoid division by zero at near-zero forward speed.
- Clamp target and smoothed curvature.
- Validate every published geometry role as finite before emitting `dataChanged`.
- A parented single-shot 500 ms stale timer in `RoadMotionViewModel` restarts
  on each valid sample. Expiry sets effective forward speed to zero and retains
  stable geometry instead of resetting or drifting.
- Do not restart the mock scenario when the user swipes between CenterHub pages.
- If the source changes later, reset timing continuity but keep QML and its bindings unchanged.

## 8. Testing Strategy

Implementation follows strict TDD.

### 8.1 Mock service tests

- Starts at the first straight stage.
- Emits equal wheel speeds during straight stages.
- Produces right-faster-than-left samples for the left curve.
- Produces left-faster-than-right samples for the right curve.
- Loops to the first stage deterministically.
- Transition samples change smoothly and remain finite.

Tests call an injected/manual advance seam; they do not wait on wall-clock timers.

### 8.2 Road ViewModel tests

- Equal wheel speeds keep curvature at zero.
- Right wheel faster produces the approved left-bend sign.
- Left wheel faster produces the approved right-bend sign.
- Smoothing approaches the target without overshoot.
- Forward motion advances segment depth.
- Stopped motion freezes segment depth.
- Passing the near boundary recycles a segment to the far boundary without changing row count.
- Dash sequencing remains stable after recycling.
- Invalid, zero, stale, and oversized elapsed samples publish no non-finite values.
- The fixed 24-row model remains stable across repeated updates.
- The same wheel-sample sequence produces identical model output regardless of the producer.

### 8.3 Integration and policy checks

- Verify CenterHub still swipes between Music and Road.
- Verify MusicPlayer state is preserved while the road page is visible.
- Verify Car/Bike/Scooter layout behavior is unchanged.
- Run the exact repository-wide Zero-JavaScript scan and classify all matches.
- Run `qt-qml-review` for the changed QML and `qt-cpp-review` for the new timer/model paths.
- Run configure, full build, all registered CTest targets, module `qmllint`, range `git diff --check`, and the 8-second offscreen smoke test.

Physical encoder validation remains a separate future phase and must not be claimed by mock-only verification.

## 9. File Impact

| File | Planned change |
|---|---|
| `src/services/MockWheelTelemetryService.{h,cpp}` | New deterministic automatic wheel-speed scenario |
| `src/viewmodels/RoadMotionViewModel.{h,cpp}` | New fixed-size road-segment model and motion logic |
| `qml/components/PerspectiveRoadView.qml` | New passive pseudo-3D single-road renderer |
| `qml/components/CenterHub.qml` | Replace the existing map page with the road view |
| `qml/components/NeonMapView.qml` | Remove after replacement |
| `src/viewmodels/MapViewModel.{h,cpp}` | Remove after replacement |
| `src/main.cpp` | Own and connect mock wheel source and road ViewModel |
| `CMakeLists.txt`, `tests/CMakeLists.txt` | Register new sources and focused test target(s) |
| `tests/` | Add deterministic mock and road-model tests; remove obsolete route-progress tests |
| `qml/Theme.qml` | Add only reusable road visual tokens required by the approved UI |
| `docs/architecture.md`, `docs/ui_ux_guidelines.md`, `docs/testing_strategy.md` | Replace the old route-loop contract after implementation is verified |
| `docs/tasks_board.md`, `docs/journal.md`, `README.md`, `AGENTS.md` | Record Phase 18 only after fresh verification |

## 10. Out of Scope

- Physical STM32 encoder parsing and field calibration.
- PWM-based direction estimation.
- GPS, IMU, Qt Location, map tiles, coordinates, or turn-by-turn navigation.
- Intersections, branching roads, lane changes, traffic, obstacles, or road names.
- Stored trajectory, trip replay, global pose, or route planning.
- Reverse travel and in-place rotation.
- Manual controls for the mock scenario.
- Shader-based rendering.

## 11. Decision Log

| Decision | Alternatives considered | Reason |
|---|---|---|
| Fixed vehicle, moving road | Moving vehicle on a static scene | Best dashboard readability and navigation-like perspective |
| Pseudo-3D perspective | Top-down 2D | Matches the premium cyberpunk dashboard and communicates forward motion |
| One lane with neon edges and dashed center | Two lanes; two-way road | Minimal, glanceable, and fits the compact CenterHub |
| Automatic repeating mock scenario | Manual UI controls; both modes | Validates motion without adding unrelated controls |
| Wheel-speed boundary | Direct left/right commands; PWM comparison | Preserves future encoder compatibility and avoids treating PWM as measured motion |
| Fixed reusable segment model | One rebuilt ShapePath; ShaderEffect | Deterministic C++ tests, bounded performance, Zero-JS compatibility |
| Replace route-loop map | Maintain both map models | Avoids contradictory concepts and dead code |
| Continue source state while page is hidden | Restart on page entry | Matches future continuous hardware telemetry and preserves current scenario state |

## 12. Success Criteria

- The Car CenterHub shows one pseudo-3D road with a fixed vehicle marker.
- The road scrolls continuously under automatic mock wheel telemetry.
- The scenario transitions smoothly through straight, left, straight, and right motion.
- All dynamic road behavior has a C++17 home; QML is passive and Zero-JavaScript compliant.
- The model remains finite, fixed-size, deterministic, and hardware-source independent.
- Music/CenterHub and vehicle-mode behavior do not regress.
- The full project golden checks and required QML/C++ reviews pass.
- Documentation clearly states that physical encoder integration remains future work.

# Design Spec: Encoder-Driven Race Kart Scene

> **AI Context**: Approved replacement design for the Phase 18 perspective-road scene. It uses deterministic mock encoder input now and preserves a C++ boundary for a later STM32 encoder adapter.
> **Date**: 2026-07-27
> **Status**: Approved by user during brainstorming; implementation not started
> **Phase**: 19

## 1. Understanding Summary

- Replace the current Perspective Road runtime feature rather than restoring the older generic map.
- Render one continuous pseudo-3D road with no centre dash, lane divider, GPS, map, route loop, button, or user-controlled steering.
- Render a lightweight rear three-quarter race kart with a chase camera; it remains near the lower screen area, moves and yaws with the turn, steers its front wheels, and stays smaller than the road scene so the road reads clearly.
- Use normalized left/right encoder motion with three response bands: straight below 5% difference, gentle drift from 5% through 20%, and a visible continuous turn above 20%.
- In the gentle band both vehicle pose and road geometry respond subtly. In the turn band both continue to follow the current encoder difference, then smoothly return to straight when the difference subsides.
- Begin with a deterministic automatic mock sequence: straight, gentle left, straight, gentle right, clear left turn, straight, clear right turn, then repeat.
- Keep all calculations, thresholds, smoothing, scene state, and mock behavior in C++17. QML is a passive Zero-JavaScript renderer.

The scene is a driving-state visualization for an encoder-equipped two-motor vehicle, not a geographic navigation map or a literal physics simulator.

## 2. Assumptions, Constraints, and Non-Goals

### 2.1 Approved assumptions

- The initial source is mock-only. Firmware, serial wire-format changes, physical encoder calibration, and field validation are out of scope.
- A later adapter supplies measured left/right encoder motion and `elapsedMs` through the same C++ contract; it replaces the mock without QML changes.
- PWM is an actuator command, never encoder feedback or odometry.
- Motion is forward-only. Reverse, in-place turns, collision, location, route history, and controls are out of scope.
- Missing, invalid, non-finite, or stale input decelerates the scene and restores yaw/curvature smoothly without resetting its visible pose.

### 2.2 Non-functional requirements

- Target smooth 60 FPS rendering. The C++ update cadence is bounded and independent of display refresh.
- Use Qt Quick vector primitives/Shapes and centralized theme tokens. Do not add a raster asset, a heavy 3D mesh, Qt Quick 3D, network access, or a per-frame QML loop.
- Every input update has bounded GUI-thread cost; no blocking I/O or per-tick QObject allocation.
- All published numeric values and path strings remain finite, bounded, and valid for QML.
- The UI remains source-swap invariant: QML does not know whether mock or future serial hardware supplied the sample.
- The project has no new privacy/security surface because it uses local telemetry only.

## 3. Decision Log

| Decision | Alternatives considered | Reason |
|---|---|---|
| Create an encoder-driving scene ViewModel | Extend `RoadMotionViewModel`; move behavior to QML | A dedicated concern keeps the old dash-segment model out of the new scene and preserves MVVM/Zero-JS boundaries. |
| Use a continuous C++-generated road path | Fixed trapezoid/dashed segments; shader | One road surface directly satisfies the no-lanes requirement and is straightforward to validate as a bounded C++ output. |
| Use a vector race-kart view | Raster image; Qt Quick 3D mesh | Vector geometry supports exposed tires, cockpit, frame, and visible steering while meeting the 60 FPS target without new runtime dependencies. |
| Use percentage bands | Fixed encoder-count thresholds; two-state turn | The percentage rule adapts to forward speed and expresses the approved straight/gentle/turn behavior. |
| Preserve Git history with forward changes | Destructive reset | The user requested a new replacement feature; commits remain recoverable and the implementation removes obsolete runtime wiring explicitly. |

## 4. Considered Approaches

### 4.1 Chosen: `EncoderDriveViewModel` and passive vector scene

`EncoderDriveViewModel` owns the measured-motion interpretation and publishes vehicle pose, front-wheel steering, plus road path properties. `EncoderDriveView.qml` consumes those properties only, while `HypercarView.qml` draws the vehicle silhouette.

This is the chosen approach because it isolates a hardware-facing driving-scene concern, permits focused C++ tests, and does not retain the dashed-road abstractions that made the previous scene read as lane-marked.

### 4.2 Rejected: extend `RoadMotionViewModel`

Reusing the existing model would reduce file count, but it couples vehicle pose to recycled dash slices and retains a representation that the user rejected. It conflicts with the project's one-ViewModel-per-concern decision.

### 4.3 Rejected: add a real 3D engine or model asset

Qt Quick 3D or a textured mesh could increase realism, but it adds dependency, asset, and performance risks without improving the encoder contract. The approved iteration needs a readable vector race kart, not physical rendering.

## 5. Architecture and Data Flow

```text
MockWheelTelemetryService
  -> wheelTelemetryUpdated(leftMotion, rightMotion, elapsedMs)
  -> EncoderDriveViewModel::updateWheelMotion(...)
  -> vehicle pose + continuous road path Q_PROPERTY values
  -> EncoderDriveView.qml + HypercarView.qml
```

The future hardware route deliberately replaces only the producer:

```text
STM32 encoder adapter
  -> wheelTelemetryUpdated(leftMotion, rightMotion, elapsedMs)
  -> unchanged EncoderDriveViewModel
  -> unchanged QML scene
```

`main.cpp` owns the mock source and ViewModel for the application lifetime, connects their typed signal/slot pair, exposes the ViewModel as one context property, and starts the mock before `app.exec()`. The future adapter follows the same ownership and source-gate discipline as existing simulator/serial telemetry.

## 6. Motion Contract

For a valid sample, C++ clamps both normalized motions to `[0, 1]`, derives mean forward motion, and computes:

```text
differenceRatio = abs(rightMotion - leftMotion) / max(meanMotion, epsilon)
direction = sign(rightMotion - leftMotion)
```

| Difference ratio | Scene response |
|---:|---|
| `< 0.05` | Straight: yaw, lateral drift, and road curvature decay toward zero. |
| `0.05–0.20` | Gentle: bounded small pose offset/yaw and low road curvature in `direction`, with the near road staying visually calm. |
| `> 0.20` | Turn: larger bounded curvature, pronounced vehicle yaw, and visible front-wheel steering continue while the difference persists; the horizon bend is amplified and the near road shifts only subtly. |

The sign convention is explicit and tested: faster right motion produces a left visual bend/yaw, while faster left motion produces a right visual bend/yaw. Smoothing is elapsed-time based, clamps long gaps, and avoids a discontinuity at either threshold. At stale timeout, forward motion goes to zero and transient yaw/curvature ease back toward straight while the last accumulated vehicle offset remains visible. The near road is intentionally subtle rather than fixed; most apparent road motion belongs to the horizon.

## 7. Components and QML Contract

### 7.1 `MockWheelTelemetryService`

The mock source is reconfigured for seven deterministic stages with injectable target values and durations. Its default stages are equal forward motion, gentle left, equal motion, gentle right, strong left, equal motion, and strong right. The default duration is demo-friendly, so a clear strong turn appears within roughly ten seconds. It contains no user control and loops automatically.

### 7.2 `EncoderDriveViewModel`

The ViewModel exposes only C++-owned observable state:

| Property | Meaning |
|---|---|
| `forwardSpeed` | Normalized accepted forward motion. |
| `vehicleLateralOffset` | Bounded horizontal car displacement in the chase frame. |
| `vehicleYawDegrees` | Bounded kart yaw for gentle drift or active turn. |
| `frontWheelSteerDegrees` | Bounded front-wheel steering angle for the race-kart visual. |
| `roadPath` | Valid normalized SVG path for the continuous filled road surface. |
| `roadEdgePath` | Valid normalized SVG path for the two road-edge strokes. |
| `roadCurvature` | Bounded signed road bend for presentation-only binding. |
| `turnState` | C++ enum: straight, gentle-left, gentle-right, turning-left, turning-right. |

The ViewModel may cache the last effective values and emit `NOTIFY` only when an observable value changed. It owns a parented stale-input timer and remains GUI-thread confined.

### 7.3 `EncoderDriveView.qml`

The QML scene uses `Shape`/`PathSvg` bindings for a single asphalt ribbon and its two outer edge glows. It has no centre line or lane representation. It binds the ViewModel paths and pose values declaratively; it performs no threshold, geometry, elapsed-time, or encoder calculation.

### 7.4 `HypercarView.qml`

The file name is retained for build stability, but the component now renders a passive rear-view race kart: exposed tires, small chassis, frame rails, cockpit/seat, steering-wheel hint, front wheels that bind to C++ steering state, and a contact shadow. It is intentionally scaled smaller than the road scene so the road stays prominent. It uses theme tokens and safe sibling-source glow where glow is needed. QML transforms bind to the ViewModel pose and front-wheel steering; no image asset or interaction is required.

## 8. Failure Handling and Edge Cases

- Reject non-positive elapsed time; cap accepted elapsed time so a stalled event loop cannot jump the scene.
- Convert non-finite wheel values to stopped motion before any division or path construction.
- Treat low mean motion as stopped and protect the ratio denominator with an epsilon.
- Clamp all pose and curvature properties before formatting paths.
- Ensure a malformed or stale sample cannot emit an invalid SVG path or cause unbounded accumulated drift.
- Repeated source start/stop does not duplicate timer connections or mock emissions.

## 9. Testing and Verification

TDD precedes each production change. `tst_encoder_drive` replaces road-specific behavior coverage and contains deterministic injected-time tests for:

- the complete seven-stage mock loop and its transitions;
- default demo timing that reaches a clear strong turn within roughly ten seconds;
- exact threshold boundaries below 5%, at 5%, at 20%, and above 20%;
- small-difference co-response of vehicle pose and road curvature;
- pronounced strong-turn yaw/curvature/front-wheel steering with a subtle near-road shift and larger horizon bend;
- sustained large-difference turn and smooth return to straight;
- direction-sign consistency, maximum clamps, invalid/extreme inputs, elapsed-time caps, stale stop, and no-change signal behavior;
- finite/bounded path output and valid turn-state transitions.

The implementation also runs configure, full build, CTest, the repository Zero-JS scan, `qt-qml-review`, module `qmllint`, `git diff --check`, and the offscreen smoke launch. Physical encoder and firmware validation remain explicitly unchecked future work.

## 10. Documentation and Migration

The implementation retires `PerspectiveRoadView.qml`, `RoadMotionViewModel`, and road-slice test/wiring only after replacement tests pass. Active architecture, UI, hardware, testing, task-board, journal, README, and `AGENTS.md` documentation change from the old dash-road wording to the encoder-driven race-kart contract. Historical Phase 18 artifacts remain historical evidence and receive only a superseded note where necessary.

The pre-existing uncommitted change in `qml/components/PerspectiveRoadView.qml` is treated as user-owned. It must be inspected and resolved deliberately before any file replacement; it is never silently overwritten.

# Encoder-Driven Hypercar Scene Implementation Plan

> **Status:** Superseded by `docs/superpowers/plans/2026-07-28-arrow-road-visual.md`.
> This document is retained as a historical record of the earlier hypercar direction.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the dashed perspective-road scene with a mock-driven, encoder-ready 3D hypercar scene on one continuous road without lane markings.

**Architecture:** `MockWheelTelemetryService` emits seven deterministic left/right normalized-motion stages. A new `EncoderDriveViewModel` owns threshold bands, smoothing, vehicle pose, turn state, and finite SVG road paths. Passive QML binds those C++ properties into an `EncoderDriveView` and reusable vector `HypercarView`; a future STM32 encoder adapter replaces only the producer.

**Tech Stack:** C++17, Qt 6.8 Core/Qml/Quick/Quick Shapes/Test, CMake, Qt Test, declarative Zero-JavaScript QML.

## Global Constraints

- Follow `AGENTS.md`: QML is passive; all encoder math, thresholds, paths, mock state, and timers belong in C++.
- Use C++17, QObject parent ownership, GUI-thread bounded work, `Q_PROPERTY` with effective-value `NOTIFY`, and `Q_INVOKABLE` only when QML needs an action.
- Do not add buttons, GPS, route history, raster assets, Qt Quick 3D, shaders, network access, or PWM-as-odometry behavior.
- Target 60 FPS by using vector `Shape`/`PathSvg` bindings and avoiding per-frame QML logic or per-tick QObject allocation.
- Motion bands are exact: difference ratio `< 0.05` straight; `>= 0.05 && <= 0.20` gentle; `> 0.20` turning. Faster right produces left visual bend/yaw; faster left produces right bend/yaw.
- Treat the existing uncommitted `qml/components/PerspectiveRoadView.qml` edit as user-owned. Inspect and obtain explicit resolution before deleting or replacing that file.
- Use TDD for every behavior: focused RED command, minimal GREEN change, focused GREEN command, then commit each task.

## File Structure

| File | Responsibility |
|---|---|
| `src/services/MockWheelTelemetryService.{h,cpp}` | Deterministic seven-stage mock source with injectable targets and bounded timer samples. |
| `src/viewmodels/EncoderDriveViewModel.{h,cpp}` | Encoder motion classification, smoothing, bounded vehicle pose, turn-state enum, and valid continuous road paths. |
| `qml/components/EncoderDriveView.qml` | Passive one-road scene consuming `EncoderDrive` properties. |
| `qml/components/HypercarView.qml` | Passive rear three-quarter cyberpunk hypercar vector silhouette. |
| `src/main.cpp` | Owns/wires `EncoderDriveViewModel` and mock source; exposes `EncoderDrive` to QML. |
| `qml/components/CenterHub.qml` | Keeps static Music page and replaces the road page with `EncoderDriveView`. |
| `qml/Theme.qml` | Central road/hypercar color and geometry tokens only. |
| `CMakeLists.txt`, `tests/CMakeLists.txt` | Replace retired sources/QML and register the new deterministic test target. |
| `tests/tst_encoder_drive.cpp` | Qt Test coverage for mock stages and driving-scene ViewModel behavior. |
| Active Markdown files | Record the new runtime architecture and keep Phase 18 historical. |

---

### Task 1: Make mock wheel telemetry express the approved seven-stage scenario

**Files:**
- Modify: `src/services/MockWheelTelemetryService.h`
- Modify: `src/services/MockWheelTelemetryService.cpp`
- Create: `tests/tst_encoder_drive.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces `MockWheelTelemetryService::wheelTelemetryUpdated(double leftMotion, double rightMotion, qint64 elapsedMs)`.
- Produces `MockWheelTelemetryConfig::targets` as `std::array<WheelMotionTarget, 7>` in this exact stage order: straight, gentle-left, straight, gentle-right, strong-left, straight, strong-right.
- `advance(qint64 elapsedMs)` accepts only `1..100` ms and emits the accepted elapsed value once per accepted call.

- [ ] **Step 1: Write failing mock-scenario tests**

Create `tests/tst_encoder_drive.cpp` with a `TestEncoderDrive` QObject. Add tests that configure 100 ms stages and 20 ms transitions, call `advance()`, and assert the seven target relationships:

```cpp
void mockVisitsGentleAndStrongStages()
{
    MockWheelTelemetryConfig config;
    config.stageDurationMs = 100;
    config.transitionDurationMs = 20;
    MockWheelTelemetryService service(config);
    QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

    // Advance into gentle-left: right > left, but ratio is <= 0.20.
    service.advance(100);
    service.advance(20);
    const auto gentleLeft = spy.takeLast();
    QVERIFY(gentleLeft.at(1).toDouble() > gentleLeft.at(0).toDouble());

    // Advance into strong-left: right > left and ratio is > 0.20.
    service.advance(100);
    service.advance(100);
    service.advance(100);
    const auto strongLeft = spy.takeLast();
    QVERIFY(strongLeft.at(1).toDouble() > strongLeft.at(0).toDouble());
}
```

Add tests named `mockStartsStraight`, `mockLoopsSevenStages`, `mockClampsOversizedElapsedTime`, `mockUsesInjectedSevenTargets`, and `mockClampsTimerIntervalToAcceptedMaximum`. Make `mockLoopsSevenStages` observe one sample from every stage and verify finite `[0, 1]` values.

- [ ] **Step 2: Register and run the RED test target**

Add a `tst_encoder_drive` executable in `tests/CMakeLists.txt` with `tst_encoder_drive.cpp`, `MockWheelTelemetryService.cpp`, and the soon-to-exist `EncoderDriveViewModel.cpp`; temporarily omit the unavailable ViewModel source so the mock-only test can compile first.

Run:

```bash
cmake -S . -B build
cmake --build build --target tst_encoder_drive -j2
./build/tests/tst_encoder_drive mockLoopsSevenStages
```

Expected: the test fails because the current four-stage configuration cannot visit the approved seven-stage sequence.

- [ ] **Step 3: Implement the seven-stage mock configuration**

In the header, replace the four-element target array with seven exact defaults:

```cpp
std::array<WheelMotionTarget, 7> targets = {{
    {1.00, 1.00}, // straight
    {0.90, 1.00}, // gentle left
    {1.00, 1.00}, // straight
    {1.00, 0.90}, // gentle right
    {0.55, 1.00}, // strong left
    {1.00, 1.00}, // straight
    {1.00, 0.55}  // strong right
}};
```

In the implementation, derive stage count from `m_config.targets.size()` and use that count for fallback target indexing, previous-stage wrap, and stage-clock wrap. Preserve finite/clamped target sanitization, timer interval limits, idempotent `start()`/`stop()`, and the 100 ms accepted sample ceiling.

- [ ] **Step 4: Run focused GREEN tests**

Run:

```bash
cmake --build build --target tst_encoder_drive -j2
./build/tests/tst_encoder_drive mockStartsStraight mockVisitsGentleAndStrongStages mockLoopsSevenStages mockClampsOversizedElapsedTime mockUsesInjectedSevenTargets mockClampsTimerIntervalToAcceptedMaximum
```

Expected: every selected test passes and no emitted wheel value is non-finite or outside `[0, 1]`.

- [ ] **Step 5: Commit Task 1**

```bash
git add src/services/MockWheelTelemetryService.h src/services/MockWheelTelemetryService.cpp tests/tst_encoder_drive.cpp tests/CMakeLists.txt
git commit -m "feat: add encoder driving mock scenario"
```

### Task 2: Add the C++ encoder-driving ViewModel with deterministic pose and path tests

**Files:**
- Create: `src/viewmodels/EncoderDriveViewModel.h`
- Create: `src/viewmodels/EncoderDriveViewModel.cpp`
- Modify: `tests/tst_encoder_drive.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes `void updateWheelMotion(double leftMotion, double rightMotion, qint64 elapsedMs)`.
- Produces `Q_PROPERTY` values: `forwardSpeed`, `vehicleLateralOffset`, `vehicleYawDegrees`, `roadCurvature`, `roadPath`, `roadEdgePath`, and `turnState`.
- Produces `enum TurnState { Straight, GentleLeft, GentleRight, TurningLeft, TurningRight }; Q_ENUM(TurnState)`.
- Uses a single-shot stale timer. Tests invoke its private slot through `QMetaObject::invokeMethod`.

- [ ] **Step 1: Write failing ViewModel tests**

Add the following focused Qt Test methods before production files exist:

```cpp
void exactThresholdBands()
{
    EncoderDriveViewModel model;
    model.updateWheelMotion(1.00, 1.04, 50);
    QCOMPARE(model.turnState(), EncoderDriveViewModel::Straight);

    model.updateWheelMotion(1.00, 1.06, 50);
    QCOMPARE(model.turnState(), EncoderDriveViewModel::GentleLeft);

    model.updateWheelMotion(1.00, 1.26, 50);
    QCOMPARE(model.turnState(), EncoderDriveViewModel::TurningLeft);
}

void smallDifferenceMovesVehicleAndRoad()
{
    EncoderDriveViewModel model;
    model.updateWheelMotion(0.90, 1.00, 100);
    QVERIFY(model.vehicleLateralOffset() < 0.0);
    QVERIFY(model.vehicleYawDegrees() < 0.0);
    QVERIFY(model.roadCurvature() < 0.0);
}
```

Also add `oppositeWheelDifferenceTurnsRight`, `sustainedTurnReturnsSmoothlyToStraight`, `pathsAreFiniteAndChangeOnlyWhenNeeded`, `invalidAndExtremeSamplesStayBounded`, `staleInputStopsWithoutResettingOffset`, and `elapsedTimeIsCapped`.

- [ ] **Step 2: Run the RED ViewModel test**

Add the new source path to `tst_encoder_drive` only after creating an empty header with the declared API. Run:

```bash
cmake --build build --target tst_encoder_drive -j2
./build/tests/tst_encoder_drive exactThresholdBands
```

Expected: compile or assertion failure because classification and properties have no implementation.

- [ ] **Step 3: Define the ViewModel header**

Create `EncoderDriveViewModel` as a `QObject` with these declarations:

```cpp
Q_PROPERTY(qreal forwardSpeed READ forwardSpeed NOTIFY forwardSpeedChanged)
Q_PROPERTY(qreal vehicleLateralOffset READ vehicleLateralOffset NOTIFY vehicleLateralOffsetChanged)
Q_PROPERTY(qreal vehicleYawDegrees READ vehicleYawDegrees NOTIFY vehicleYawDegreesChanged)
Q_PROPERTY(qreal roadCurvature READ roadCurvature NOTIFY roadCurvatureChanged)
Q_PROPERTY(QString roadPath READ roadPath NOTIFY roadPathChanged)
Q_PROPERTY(QString roadEdgePath READ roadEdgePath NOTIFY roadEdgePathChanged)
Q_PROPERTY(TurnState turnState READ turnState NOTIFY turnStateChanged)
```

Declare the read accessors, `updateWheelMotion`, signals, `handleStaleTimeout`, `updateRoadPaths`, and helpers `classifyTurn`, `formatRoadPath`, and `formatRoadEdgePath`.

- [ ] **Step 4: Implement bounded motion and road paths**

Implement these rules in `EncoderDriveViewModel.cpp`:

```cpp
const double meanMotion = (leftMotion + rightMotion) / 2.0;
const double ratio = std::abs(rightMotion - leftMotion)
    / (std::max)(meanMotion, kMinimumMotion);
const double signedDifference = rightMotion - leftMotion;
```

Clamp finite input to `[0.0, 1.0]`, cap elapsed time at 100 ms, classify exact bands, and use elapsed-time interpolation for `vehicleLateralOffset`, `vehicleYawDegrees`, and `roadCurvature`. Adopt this sign mapping: positive `signedDifference` yields negative lateral offset/yaw/curvature (left visual response), negative difference yields positive response (right visual response).

Build a continuous normalized SVG path from four C++-calculated control points: horizon left/right, near left/right. Clamp every X/Y value before `QString::asprintf` formatting. Generate `roadPath` as one closed ribbon and `roadEdgePath` as two independent open edge subpaths. Cache strings and emit a signal only if the effective string changes.

- [ ] **Step 5: Run focused GREEN tests**

Run:

```bash
cmake --build build --target tst_encoder_drive -j2
./build/tests/tst_encoder_drive exactThresholdBands smallDifferenceMovesVehicleAndRoad oppositeWheelDifferenceTurnsRight sustainedTurnReturnsSmoothlyToStraight pathsAreFiniteAndChangeOnlyWhenNeeded invalidAndExtremeSamplesStayBounded staleInputStopsWithoutResettingOffset elapsedTimeIsCapped
```

Expected: all selected tests pass; every published path contains only finite decimal coordinates and no test observes a property change for identical stopped input.

- [ ] **Step 6: Commit Task 2**

```bash
git add src/viewmodels/EncoderDriveViewModel.h src/viewmodels/EncoderDriveViewModel.cpp tests/tst_encoder_drive.cpp tests/CMakeLists.txt
git commit -m "feat: model encoder-driven vehicle scene"
```

### Task 3: Resolve the user-owned old QML edit and render the passive hypercar scene

**Files:**
- User-owned overlap: `qml/components/PerspectiveRoadView.qml`
- Create: `qml/components/EncoderDriveView.qml`
- Create: `qml/components/HypercarView.qml`
- Modify: `qml/Theme.qml`
- Modify: `qml/components/CenterHub.qml`
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces and consumes the `EncoderDrive` context property in this task.
- `EncoderDriveView.qml` binds `EncoderDrive.roadPath`, `EncoderDrive.roadEdgePath`, `EncoderDrive.vehicleLateralOffset`, and `EncoderDrive.vehicleYawDegrees`.
- `HypercarView.qml` exposes only declarative `property real lateralOffset` and `property real yawDegrees` presentation inputs.

- [ ] **Step 1: Resolve the user-owned overlap before modifying it**

Run:

```bash
git diff -- qml/components/PerspectiveRoadView.qml
```

If the user asks to preserve content, copy its approved intent into the replacement deliberately. If the user confirms retirement, delete the old component using `apply_patch`; do not use `git checkout`, `git reset`, or an unreviewed overwrite. Do not continue this task without that explicit resolution.

- [ ] **Step 2: Add passive QML target structure and run the RED policy check**

Create the new QML files using only imports, items, declarative bindings, `Shape`, `ShapePath`, `PathSvg`, `Behavior`, and `NumberAnimation`. Before any C++-backed bindings exist, run:

```bash
rg -n '\\bMath\\.|on[A-Z][A-Za-z]+\\s*:\\s*\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml/components/EncoderDriveView.qml qml/components/HypercarView.qml
```

Expected: no executable JavaScript match. A QML load check is expected to fail until Task 4 exposes `EncoderDrive` and CMake lists the files.

- [ ] **Step 3: Implement `HypercarView.qml` as vector-only passive presentation**

Use `Shape` paths or `Rectangle`/`Gradient` primitives for these visible layers, ordered back to front: contact shadow, rear wheels, low wide body, rear bumper, cabin/rear glass, spoiler, tail lamps, and cyan highlight. Bind its root `x` and `rotation` to `lateralOffset` and `yawDegrees`; do not calculate transforms in QML. Use `Theme` tokens for body, glass, tail-lamp, shadow, and highlight colors.

- [ ] **Step 4: Implement `EncoderDriveView.qml`, runtime wiring, and CenterHub integration**

Bind a `PathSvg` fill to `EncoderDrive.roadPath` and a sibling `PathSvg` stroke to `EncoderDrive.roadEdgePath`. Keep the road as one filled continuous surface: no `Repeater`, center marking, lane divider, map grid, or marker arrow. Anchor `HypercarView` near the lower chase-camera frame and pass through the ViewModel pose properties. In `CenterHub.qml`, replace `PerspectiveRoadView {}` with `EncoderDriveView {}` while preserving static MusicPlayer lifetime, SwipeView behavior, and page indicator.

In `main.cpp`, create `EncoderDriveViewModel encoderDriveVm`, connect the existing mock signal to `EncoderDriveViewModel::updateWheelMotion`, and expose `EncoderDrive` before `engine.loadFromModule()`. In `CMakeLists.txt`, add the new ViewModel source and both new QML files while retaining the old road files until Task 4 retires them.

- [ ] **Step 5: Run QML lint and visual GREEN checks**

Run:

```bash
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py qml/components/EncoderDriveView.qml qml/components/HypercarView.qml qml/components/CenterHub.qml qml/Theme.qml
cmake -S . -B build
cmake --build build -j2
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
```

Expected: deterministic lint reports no Zero-JS finding; the configured build succeeds with both old and replacement sources present; module `qmllint` exits 0.

- [ ] **Step 6: Commit Task 3**

```bash
git add qml/components/EncoderDriveView.qml qml/components/HypercarView.qml qml/components/CenterHub.qml qml/Theme.qml qml/components/PerspectiveRoadView.qml
git commit -m "feat: render encoder-driven hypercar scene"
```

### Task 4: Retire the old dashed-road runtime only after the replacement is green

**Files:**
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`
- Delete: `src/viewmodels/RoadMotionViewModel.h`
- Delete: `src/viewmodels/RoadMotionViewModel.cpp`
- Delete: `tests/tst_road_motion.cpp`
- Delete: `qml/components/PerspectiveRoadView.qml` only after Task 3 user-overlap resolution

**Interfaces:**
- Keeps the QML context property `EncoderDrive` bound to one stack-owned `EncoderDriveViewModel`.
- Removes the `RoadMotion` context property and all `RoadMotionViewModel` CMake/test references.
- Keeps `MockWheelTelemetryService::wheelTelemetryUpdated` as the source-independent signal contract.

- [ ] **Step 1: Write failing integration expectations**

Add tests in `tst_encoder_drive.cpp` that instantiate the mock plus ViewModel and connect the exact typed signal/slot pair:

```cpp
QObject::connect(&mock, &MockWheelTelemetryService::wheelTelemetryUpdated,
                 &scene, &EncoderDriveViewModel::updateWheelMotion);
mock.advance(50);
QVERIFY(scene.forwardSpeed() > 0.0);
QVERIFY(!scene.roadPath().isEmpty());
```

Add a source-retirement check command to the task report:

```bash
rg -n 'RoadMotion|RoadMotionViewModel|PerspectiveRoadView|centerLineVisible' src qml tests CMakeLists.txt
```

Expected before migration: matches exist.

- [ ] **Step 2: Run the RED integration/retirement check**

Run:

```bash
cmake --build build --target tst_encoder_drive -j2
./build/tests/tst_encoder_drive mockSignalFeedsEncoderScene
rg -n 'RoadMotion|RoadMotionViewModel|PerspectiveRoadView|centerLineVisible' src qml tests CMakeLists.txt
```

Expected: the retirement command still reports the old runtime references before deletion.

- [ ] **Step 3: Remove obsolete runtime and CMake wiring**

Remove the `RoadMotionViewModel` include, object, connection, and `RoadMotion` context property from `main.cpp`. In root CMake, remove the old ViewModel sources and QML file, leaving the `EncoderDriveViewModel.{h,cpp}`, `EncoderDriveView.qml`, and `HypercarView.qml` entries added in Task 3. In test CMake, keep only `tst_encoder_drive` for this concern and remove the old road target.

- [ ] **Step 4: Retire old runtime artifacts after green replacement**

Use `apply_patch` deletion for the road ViewModel/test and, only after Task 3 resolution, the old QML component. Do not alter historical specs/plans. Confirm no active runtime reference remains with the exact `rg` command from Step 1.

- [ ] **Step 5: Run integration GREEN checks**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
./build/tests/tst_encoder_drive mockSignalFeedsEncoderScene
rg -n 'RoadMotion|RoadMotionViewModel|PerspectiveRoadView|centerLineVisible' src qml tests CMakeLists.txt
```

Expected: build succeeds, the signal-to-scene test passes, and `rg` returns no match.

- [ ] **Step 6: Commit Task 4**

```bash
git add src/main.cpp CMakeLists.txt tests/CMakeLists.txt src/viewmodels qml/components tests
git commit -m "refactor: replace dashed road with encoder drive scene"
```

### Task 5: Synchronize active documentation and close verification

**Files:**
- Modify: `AGENTS.md`
- Modify: `README.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`

**Interfaces:**
- Documents `MockWheelTelemetryService -> EncoderDriveViewModel -> EncoderDriveView/HypercarView`.
- Documents 5%/20% response bands and the hard rule that PWM is not encoder feedback.
- Records physical encoder firmware integration as future unchecked work.

- [ ] **Step 1: Update active architecture/UI/hardware/testing truth**

Replace old dash-road wording with: one continuous road; no lane divider; C++ pose/path contract; vector hypercar; seven-stage mock; future measured encoder adapter; and PWM prohibition. Change test-target tables from `tst_road_motion` to `tst_encoder_drive`. Keep Phase 18 prose as historical, marked superseded by Phase 19.

- [ ] **Step 2: Record final verification evidence in task board and journal**

Add a Phase 19 checklist with implementation, tests, Zero-JS/QML review, documentation, and full verification items. Leave physical encoder/firmware validation unchecked. Record commands, exact CTest count, comment-only Zero-JS matches, lint result, smoke timeout result, and review outcome only after they occur.

- [ ] **Step 3: Run the full verification matrix**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
rg -n '\\bMath\\.|on[A-Z][A-Za-z]+\\s*:\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml -g '*.qml'
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
git diff --check
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator
```

Expected: configure/build exit 0; every registered CTest target passes; Zero-JS matches are only manually classified comments; both QML lint commands exit 0; diff check is clean; smoke exits 124 only because the application remains running, with no QML load/runtime error.

- [ ] **Step 4: Run focused reviews and resolve findings**

Run the project `qt-cpp-review` workflow for changed C++ and the `qt-qml-review` workflow for changed QML. Fix every Critical or Important finding with a regression test where behavior is affected, then rerun the full matrix.

- [ ] **Step 5: Commit Task 5**

```bash
git add AGENTS.md README.md docs/architecture.md docs/ui_ux_guidelines.md docs/hardware_integration.md docs/testing_strategy.md docs/tasks_board.md docs/journal.md
git commit -m "docs: record encoder-driven hypercar scene"
```

## Plan Self-Review

- **Spec coverage:** Task 1 implements the automatic seven-stage mock; Task 2 implements exact 5%/20% bands, sign, stale/invalid handling, and finite C++ paths; Task 3 implements and wires the passive one-road hypercar scene; Task 4 retires old runtime code after replacement checks; Task 5 updates active docs and runs all required checks.
- **Safety coverage:** Task 3 explicitly blocks on the user-owned uncommitted QML edit. No task uses destructive Git reset/checkout; retirement occurs only after tested replacement.
- **Type consistency:** The mock signal, ViewModel slot, context property, C++ `Q_PROPERTY` names, QML bindings, and test target names are defined before consumers use them.
- **Placeholder scan:** No unresolved work markers or unspecified validation behavior remain; every failure mode, command, threshold, and runtime boundary has an assigned task.

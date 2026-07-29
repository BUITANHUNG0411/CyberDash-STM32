# Cockpit Context Rail Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a passive, glanceable rail that exposes vehicle, drive, theme, telemetry source, and Safety Lab availability without changing vehicle or hardware behavior.

**Architecture:** A focused `CockpitContextViewModel` observes existing UI ViewModels and receives the already-authoritative serial connection state from `main.cpp`. A new passive QML component formats no data and has no interaction; it binds directly to the new ViewModel and existing `Theme` tokens.

**Tech Stack:** Qt 6.8+, Qt Quick/QML, C++17, Qt Test, CMake.

## Global Constraints

- QML is passive: no imperative JavaScript, local interaction state, timers, labels, or calculations.
- Preserve the UART protocol, `SerialService`, fallback behavior, Music, Parking Assist, vehicle morphs, and Safety Lab contracts.
- No dependencies, new services, simulated driving/sensor/ADAS behavior, or hardware requirement.
- Reuse existing `Theme.qml` colors, typography, spacings, radii, and durations.
- `CockpitContextViewModel` is GUI-thread-only and emits effective-value notifications only.

---

### Task 1: Test and implement the focused context ViewModel

**Files:**
- Create: `src/viewmodels/CockpitContextViewModel.h`
- Create: `src/viewmodels/CockpitContextViewModel.cpp`
- Create: `tests/tst_cockpit_context.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `VehicleModeViewModel`, `DriveModeViewModel`, `ThemeViewModel`, `ParkingAssistViewModel`, `SafetyScenarioViewModel`.
- Produces: labels for vehicle mode, drive mode, theme, source, and Safety Lab; `setHardwareConnected(bool)` for `main.cpp` only.

- [x] Write failing Qt Test cases for defaults, vehicle/drive/theme transitions, source flip, Safety Lab states, and duplicate-notification suppression.
- [x] Run `cmake --build build -j2 --target tst_cockpit_context`; expect compilation failure because the target and class are absent.
- [x] Implement the minimal QObject ViewModel with `Q_PROPERTY(QString ...)` labels, a `Q_PROPERTY(bool safetyLabAvailable ...)`, and effective-value signals.
- [x] Register the test target and required source files in `tests/CMakeLists.txt`.
- [x] Build and run `ctest --test-dir build -R tst_cockpit_context --output-on-failure`; expect pass.

### Task 2: Wire the passive rail into the application

**Files:**
- Create: `qml/components/CockpitContextRail.qml`
- Modify: `src/main.cpp`
- Modify: `qml/screens/DashboardScreen.qml`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: context property `CockpitContext` and existing `ThemeController`/`Theme` tokens.
- Produces: a non-interactive rail under `topBar`, hidden during boot and present in Car, Bike, and Scooter layouts.

- [x] Bind `CockpitContextViewModel::setHardwareConnected` into the existing `SerialService::connectionStatusChanged` lambda; expose one stack-owned `CockpitContext` property.
- [x] Create five static passive QML pills that bind only C++ labels and existing theme tokens.
- [x] Anchor the component below `topBar`; use `Theme.spaceLg`, `Theme.spaceSm`, `Theme.textXs`, `Theme.radiusPill`, glass tokens, and existing boot opacity/duration.
- [x] Register C++ and QML sources in the application CMake target.
- [x] Build and run the focused test target; expect pass.

### Task 3: Verify regression boundaries and QML policy

**Files:**
- Verify only: changed files, `tests/`, and QML module output.

- [x] Run configure, full build, and full CTest.
- [x] Run the repository Zero-JS scan and the project QML review linter.
- [x] Run an offscreen smoke test; accept timeout 124 only when output has no QML/runtime error.
- [x] Review the diff for source-swap invariance and absence of changes to Serial/Parking/Safety behavior.

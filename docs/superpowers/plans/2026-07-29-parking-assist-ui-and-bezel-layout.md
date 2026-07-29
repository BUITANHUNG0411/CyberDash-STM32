# Parking Assist UI and Bezel Layout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task.

**Goal:** Upgrade the rear parking panel into a glanceable OEM-style distance display and widen the double-arch layout so the center panel remains visually separated from both gauges.

**Architecture:** Keep all distance semantics in `ParkingAssistViewModel` and expose one presentation-safe proximity progress value plus segment count. QML remains a passive view. Move the two gauge centers and the bezel path endpoints outward through centralized `Theme.qml` geometry tokens; give `centerPanel` positive spacing instead of overlap margins.

**Tech Stack:** Qt 6.8+, Qt Quick/QML, C++17, QtTest, CMake.

## Global Constraints

- Zero JavaScript in QML; no functions, control flow, local mutation, or calculations that belong in C++.
- C++17 MVVM; QML consumes `Q_PROPERTY` values and direct `Q_INVOKABLE` calls only.
- Follow the existing theme tokens and double-arch silhouette; do not add camera/map/3D artwork.
- New C++ behavior follows TDD: failing focused test first, then minimal implementation.
- Run configure, full build, all CTest targets, Zero-JS scan, QML lint/review, and offscreen smoke before completion.

---

### Task 1: Add presentation-safe proximity progress

**Files:**
- Modify: `src/viewmodels/ParkingAssistViewModel.h`
- Modify: `src/viewmodels/ParkingAssistViewModel.cpp`
- Test: `tests/tst_parking_assist.cpp`

**Interfaces:**
- Produces `Q_PROPERTY(int proximitySegments READ proximitySegments NOTIFY proximitySegmentsChanged)` with values 0–8, where 0 is unavailable, 1–8 increases as the obstacle gets closer.
- Produces `Q_PROPERTY(real proximityProgress READ proximityProgress NOTIFY proximityProgressChanged)` with values 0.0–1.0, where 0.0 is far/unavailable and 1.0 is the stop threshold.

- [x] Add focused assertions for clear, caution, stop, and unavailable samples; verify unchanged samples do not duplicate notifications.
- [x] Run `cmake --build build --target tst_parking_assist` and the focused test; observe RED before implementation.
- [x] Implement clamped derivation in the existing sample update path without changing the sensor contract or thresholds.
- [x] Re-run the focused test GREEN, then run the full parking target.

### Task 2: Rebuild Parking Assist as a glanceable OEM panel

**Files:**
- Modify: `qml/components/ParkingAssistView.qml`
- Modify: `qml/Theme.qml` only if a missing geometry/color token is required

**Interfaces:**
- Consumes `ParkingAssist.distanceText`, `statusText`, `sensorAvailable`, `proximityLevel`, `proximityProgress`, and `proximitySegments`.
- Keeps `CenterHub.qml` and all C++ transport code unchanged.

- [x] Replace the three static bars with a compact two-row header (`REAR PARK ASSIST`, `ULTRASONIC ONLINE`), large distance readout, segmented proximity track, and a small obstacle block that moves toward the bumper using `proximityProgress`.
- [x] Bind all colors to `Theme` tokens; pulse only the STOP state and animate numeric/position properties, never text.
- [x] Keep the component passive and avoid QML JavaScript or imperative state.
- [x] Run the repository Zero-JS scan and `qt_qml_lint.py all qml` after the component change.

### Task 3: Widen the frame and restore center-panel clearance

**Files:**
- Modify: `qml/Theme.qml`
- Modify: `qml/Main.qml`
- Modify: `qml/screens/DashboardScreen.qml`

**Interfaces:**
- Adds centralized geometry tokens for the widened bezel endpoints, gauge centers, and center-panel clearance.
- Does not change ViewModel or transport behavior.

- [x] Add tokens for the widened left/right arch x positions, gauge inset centers, and a positive center-panel gap.
- [x] Bind both `PathSvg` paths in `Main.qml` to the new tokens so the bezel extends around the outer gauge edges.
- [x] Move the gauge centers outward through `Theme.gaugeInsetLeft/Right` and replace `centerPanel` negative margins with the positive gap token.
- [x] Preserve anchors/layout rules and vehicle-mode transitions; run QML lint and an offscreen smoke check to catch clipping or binding errors.

### Task 4: Synchronize project documentation

**Files:**
- Modify: `AGENTS.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`

- [x] Record the distance-progress presentation contract and the widened-bezel/positive-gap decision.
- [x] Mark the new Phase 19 items complete only after verification evidence exists.
- [x] Ensure no retired map or obsolete parking-panel wording remains.

### Task 5: Full verification, commit, and push

- [x] Run `cmake -S . -B build`.
- [x] Run `cmake --build build -j2`.
- [x] Run `ctest --test-dir build --output-on-failure`.
- [x] Run the exact Zero-JS scan and `python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml`.
- [x] Run `QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator`; treat exit 124 as expected only if there are no QML/runtime errors.
- [x] Run `git diff --check`, inspect the diff, commit with a focused message (`17f4b55`), and push `main` to `origin` successfully.
- [x] Push succeeded; no manual push fallback was required.

### Post-plan visual correction

The widened bezel and positive CenterHub clearance from Task 3 were intentionally reverted after
visual review. The active baseline restores the original compact geometry: arch endpoints `270/890`,
horizontal radius `250`, gauge centres `260/880`, and CenterHub margins `-20`. Parking Assist and
its two-row header remain unchanged.

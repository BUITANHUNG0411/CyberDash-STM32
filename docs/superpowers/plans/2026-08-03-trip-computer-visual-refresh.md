# Trip Computer Visual Refresh Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refresh the Trip Computer hierarchy and reduce the Parking Assist distance readout scale without changing the C++ data contract.

**Architecture:** Keep both pages as passive QML views backed by the existing `TripComputerViewModel` and `ParkingAssistViewModel`. Add only centralized typography tokens in `Theme.qml`; use a source-contract test to protect the visual structure and zero-JavaScript boundary.

**Tech Stack:** Qt 6.8+, Qt Quick/QML, C++17, Qt Test, CMake.

## Global Constraints

- QML remains passive: no executable JavaScript, local mutation, loops, or calculations.
- Existing C++ display strings and `resetTrip()` invokable remain the only Trip data/action boundary.
- Use `Theme.qml` tokens for new visual constants.
- Preserve CenterHub navigation, parking safety override, telemetry, and double-arch geometry.
- Do not add dependencies, delete files, or commit changes.

---

### Task 1: Add the failing visual-contract test

**Files:**
- Modify: `tests/main.cpp`
- Test: `qml/components/TripComputerView.qml`, `qml/components/ParkingAssistView.qml`

- [ ] Add `testTripComputerVisualContract()` that loads both QML source files, asserts the Trip hero/stat/reset markers and dedicated theme tokens, asserts Parking uses `Theme.parkingDistanceDisplay`, and rejects executable JavaScript patterns.
- [ ] Run `cmake --build build --target tst_viewmodels -j2 && ctest --test-dir build -R tst_viewmodels --output-on-failure`.
- [ ] Confirm the test fails because the new visual markers/tokens do not exist yet.

### Task 2: Add centralized visual tokens

**Files:**
- Modify: `qml/Theme.qml`

- [ ] Add `parkingDistanceDisplay`, `tripHeroDisplay`, `tripMetricDisplay`, and `tripMetaDisplay` as readonly typography tokens near the existing display sizes.
- [ ] Keep values sized for the compact panel: parking smaller than `displayMd`, Trip hero between `displaySm` and `displayMd`, secondary stats below `displaySm`.

### Task 3: Recompose the two QML views

**Files:**
- Modify: `qml/components/ParkingAssistView.qml`
- Modify: `qml/components/TripComputerView.qml`

- [ ] Bind Parking Assist's distance readout to `Theme.parkingDistanceDisplay`.
- [ ] Replace Trip's flat column with the compact glass data-card hierarchy defined in the spec.
- [ ] Use only existing `TripComputer.tripDisplay`, `TripComputer.odoDisplay`, `TripComputer.avgSpeedDisplay`, and `TripComputer.resetTrip()` interfaces.
- [ ] Keep all handlers as direct C++ invokable calls and all animation declarative.

### Task 4: Verify the implementation

**Files:**
- Verify: `tests/main.cpp`, `qml/Theme.qml`, `qml/components/ParkingAssistView.qml`, `qml/components/TripComputerView.qml`

- [ ] Run the focused source-contract test and confirm it passes.
- [ ] Run `cmake -S . -B build`.
- [ ] Run `cmake --build build -j2`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Run the repository zero-JavaScript scan and the `qt-qml-review` workflow/linter.
- [ ] Run `QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator`, inspect output, and treat exit 124 as expected only when no runtime errors appear.

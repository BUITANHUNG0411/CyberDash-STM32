# OEM Instrument Cluster Skin Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add shared physical cluster chrome and page-specific status accents inspired by the provided OEM instrument-cluster reference.

**Architecture:** Create a passive `InstrumentFrame.qml` overlay component. Integrate it into `GlassPanel.qml` and the Music player, while binding Parking's accent to its existing presentation color. Add a small declarative LED rail to `CenterHub.qml`; C++ page and proximity state remain the only source of truth.

**Tech Stack:** Qt 6.8+, Qt Quick/QML, C++17, Qt Test, CMake.

## Global Constraints

- QML is a passive view with zero executable JavaScript.
- The application remains a single Car dashboard with the existing double-arch geometry.
- `CenterHub.qml` keeps the stable `StackLayout` and existing C++ navigation contract.
- No new ViewModel, service, dependency, UART field, page, or vehicle mode is added.
- New visual constants live in `qml/Theme.qml`.
- Do not modify or overwrite the pre-existing user change in `AGENTS.md`.
- Do not commit changes.

---

### Task 1: Add the failing OEM cluster source-contract test

**Files:**
- Modify: `tests/main.cpp`
- Test: `CMakeLists.txt`, `qml/components/InstrumentFrame.qml`, `qml/components/GlassPanel.qml`, `qml/components/MusicPlayer.qml`, `qml/components/ParkingAssistView.qml`, `qml/components/CenterHub.qml`

**Interfaces:**
- The test requires the QML module to register `InstrumentFrame.qml`.
- The test requires `GlassPanel.qml` to expose `accentColor` and instantiate `InstrumentFrame`.
- The test requires Music and Parking to wire the shared frame.
- The test requires CenterHub to expose `clusterStatusLeds` and bind active page accents declaratively.

- [ ] Add `testOemClusterSkinContract()` to `tests/main.cpp`.
- [ ] Load the six source files with `QFINDTESTDATA`.
- [ ] Assert the module registration, `accentColor` property, `InstrumentFrame` usage, Parking accent binding, LED id, and Theme token usage.
- [ ] Assert `InstrumentFrame.qml` contains no executable JavaScript keyword.
- [ ] Run `cmake --build build --target tst_viewmodels -j2 && ctest --test-dir build -R tst_viewmodels --output-on-failure`.
- [ ] Confirm the test fails because `InstrumentFrame.qml`, its registration, and the new CenterHub contract do not yet exist.

### Task 2: Add centralized cluster skin tokens and frame component

**Files:**
- Modify: `qml/Theme.qml`
- Create: `qml/components/InstrumentFrame.qml`
- Modify: `CMakeLists.txt`

- [ ] Add Theme tokens for frame inset/line widths, highlight ratio/opacity, and CenterHub status LED dimensions.
- [ ] Keep the legibility contract at `clusterFrameLineWidth: 2`, `clusterFrameHighlightRatio: 0.48`, `centerStatusLedWidth: 32`, and `centerStatusLedHeight: 6` for the current dashboard scale.
- [ ] Create `InstrumentFrame.qml` as a transparent `Item` with typed `accentColor` property.
- [ ] Render only declarative rectangles: outer border, inner lip, and short top accent highlight.
- [ ] Register `InstrumentFrame.qml` in the existing `com.showcase` QML module.
- [ ] Keep the component free of MouseArea, timers, JavaScript, and backend references.

### Task 3: Integrate shared frame and page accents

**Files:**
- Modify: `qml/components/GlassPanel.qml`
- Modify: `qml/components/MusicPlayer.qml`
- Modify: `qml/components/ParkingAssistView.qml`
- Modify: `qml/components/TripComputerView.qml`
- Modify: `qml/components/CenterHub.qml`

- [ ] Add `property color accentColor: Theme.accentCyan` to `GlassPanel.qml` and place an `InstrumentFrame` above panel content.
- [ ] Add a cyan `InstrumentFrame` overlay to MusicPlayer, which intentionally does not inherit `GlassPanel`.
- [ ] Bind Parking's `GlassPanel.accentColor` to `root.proximityColor`.
- [ ] Keep Trip's frame cyan and preserve its existing refreshed hierarchy.
- [ ] Add `root.activePageAccent` as a declarative ternary: Parking uses warning-red at critical proximity, parking amber otherwise, and cyan for Music/Trip.
- [ ] Add a three-LED `clusterStatusLeds` rail above the existing tabs, with LED colors bound to the active page and existing Parking state.
- [ ] Move the existing StackLayout top margin down only by centralized LED/tab geometry tokens; do not change bezel or gauge anchors.

### Task 4: Verify the cluster skin

**Files:**
- Verify: `qml/Theme.qml`, `qml/components/InstrumentFrame.qml`, `qml/components/GlassPanel.qml`, `qml/components/MusicPlayer.qml`, `qml/components/ParkingAssistView.qml`, `qml/components/TripComputerView.qml`, `qml/components/CenterHub.qml`, `tests/main.cpp`, `CMakeLists.txt`

- [ ] Run the focused source-contract test and confirm it passes.
- [ ] Run `cmake -S . -B build`.
- [ ] Run `cmake --build build -j2`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Run `rg -n '\\bMath\\.|on[A-Z][A-Za-z]+\\s*:\\s*\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml -g '*.qml'` and classify only comments as non-executable.
- [ ] Run `python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml`.
- [ ] Run `QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator`; inspect runtime output and treat exit 124 as expected timeout only when no QML/runtime errors appear.
- [ ] Run `git diff --check` and confirm no whitespace errors.

# Center Hub Manual Swipe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Restore horizontal mouse drag navigation between Music Player and Distance Warning without losing automatic critical-distance handoff.

**Architecture:** Keep `StackLayout` as the stable two-page container. A QML `DragHandler` forwards pointer lifecycle and translation to `CenterHubViewModel`; C++ applies the threshold, page validation, and safety override. The warning page remains renderable for manual inspection at every sensor state.

**Tech Stack:** Qt 6 Quick/QML, Qt Test, C++17, CMake.

## Global Constraints

- Zero JavaScript in QML.
- MVVM: gesture decisions and thresholds live in C++.
- Keep Simulator ↔ Serial source-swap invariance.
- Preserve automatic Parking Assist selection only for live distance `<30 cm`.

### Task 1: Test CenterHub gesture contract

**Files:**
- Modify: `tests/tst_parking_assist.cpp`
- Modify: `src/viewmodels/CenterHubViewModel.h` (test-visible API declaration)

- [ ] Add tests for left/right committed drags, sub-threshold drags, invalid page requests, and critical safety override.
- [ ] Run `cmake --build build --target tst_parking_assist` and confirm the new tests fail before production implementation.

### Task 2: Implement C++ page and gesture state

**Files:**
- Modify: `src/viewmodels/CenterHubViewModel.h`
- Modify: `src/viewmodels/CenterHubViewModel.cpp`

- [ ] Add invokables for pointer active state and translation updates plus a validated manual page request.
- [ ] Use an 80 px threshold; left selects `ParkingPage`, right selects `MusicPage`.
- [ ] Reject manual Music selection while `criticalProximity()` is true.
- [ ] Run the focused test target until green.

### Task 3: Bind the passive QML gesture

**Files:**
- Modify: `qml/components/CenterHub.qml`
- Modify: `qml/components/ParkingAssistView.qml`

- [ ] Add a null-target `DragHandler` over the hub with takeover permissions for nested controls.
- [ ] Forward `active` and `translation.x` through direct invokable calls only.
- [ ] Remove the warning view's critical-only visibility gate so StackLayout can display manual clear/caution/stale states.

### Task 4: Documentation and verification

**Files:**
- Modify: `AGENTS.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`

- [ ] Document manual swipe plus critical safety precedence.
- [ ] Run configure, build, CTest, Zero-JS scan, QML lint, module qmllint, smoke, and `git diff --check`.
- [ ] Review the final diff and prepare a commit summary.

# Arrow Road Visual Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the vehicle visual with a centered arrow that follows the road direction while the road itself keeps the encoder-driven curved path.

**Architecture:** `EncoderDriveViewModel` continues to own road curvature, yaw, and finite road geometry in C++. QML becomes a passive road-and-arrow presentation: the road remains a single continuous surface, and the arrow sits centered on the road, rotating with the current turn direction. No new backend source or runtime branch is introduced.

**Tech Stack:** Qt 6/QML Shapes, C++17, Qt Test, CMake.

## Global Constraints

- Zero imperative JavaScript in QML.
- Modern C++17 only.
- MVVM: visual state comes from C++ `Q_PROPERTY` values.
- Keep the mock/hardware source-swap boundary unchanged.
- Do not stage `.superpowers/`.

---

### Task 1: Keep the road contract and add arrow-facing regression coverage

**Files:**
- Modify: `tests/tst_encoder_drive.cpp`

**Interfaces:**
- Consumes: `EncoderDriveViewModel::vehicleYawDegrees()`, `EncoderDriveViewModel::roadCurvature()`.
- Produces: stronger regression coverage that the C++ road signal still bends and the turn sign still matches the road direction.

- [x] Add a focused test proving a left-turn sample produces negative yaw/curvature and a right-turn sample produces positive yaw/curvature.
- [x] Add a focused test proving the road path bends at the horizon while remaining finite and continuous.
- [x] Run `cmake --build build --target tst_encoder_drive -j2 && ./build/tests/tst_encoder_drive`.

### Task 2: Replace the vehicle with a centered arrow

**Files:**
- Modify: `qml/components/HypercarView.qml`
- Modify: `qml/components/EncoderDriveView.qml`
- Modify: `qml/Theme.qml`

**Interfaces:**
- Consumes: `EncoderDrive.roadPath`, `EncoderDrive.roadEdgePath`, `EncoderDrive.vehicleYawDegrees`, and optionally `EncoderDrive.roadCurvature`.
- Produces: a centered arrow that sits on the road and rotates with the road direction while the road remains the main moving surface.

- [x] Remove the vehicle silhouette and replace it with a single arrow-shaped vector marker.
- [x] Keep the arrow centered on the road rather than offset to a vehicle lane.
- [x] Rotate the arrow to match the current road direction using the C++ turn state already exposed to QML.
- [x] Keep all geometry declarative and Zero-JS compliant.

### Task 3: Documentation sync

**Files:**
- Modify: `README.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`
- Modify: `docs/superpowers/specs/2026-07-27-encoder-driven-hypercar-design.md`

**Interfaces:**
- Consumes: implemented arrow-road visual.
- Produces: active Markdown that describes a road-following arrow instead of a vehicle visual.

- [x] Update active wording from vehicle/hypercar to arrow where relevant.
- [x] Record that the arrow is centered on the road and rotates with the road direction.
- [x] Keep hardware caveats explicit: PWM is not encoder feedback.

### Task 4: Verification and commit

**Files:**
- All changed files.

**Interfaces:**
- Consumes: Tasks 1-3.
- Produces: verified commit.

- [x] Run `cmake -S . -B build`.
- [x] Run `cmake --build build -j2`.
- [x] Run `ctest --test-dir build --output-on-failure`.
- [x] Run the Zero-JS scan from `docs/testing_strategy.md`.
- [x] Run `python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml`.
- [x] Run `/usr/lib/qt6/bin/qmllint --module com.showcase -I build`.
- [x] Run `git diff --check`.
- [x] Run `timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator`.
- [x] Stage all intended files except `.superpowers/`.
- [x] Commit with `feat: replace vehicle visual with road arrow`.

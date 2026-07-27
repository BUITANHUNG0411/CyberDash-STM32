# Race Kart Encoder Visual Implementation Plan

> **Status:** Superseded by `docs/superpowers/plans/2026-07-28-arrow-road-visual.md`.
> This plan is kept only as historical context for the brief race-kart branch that was replaced by the centered arrow scene.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current hypercar-like drawing with a cheaper, race-kart-style vector vehicle that visibly bo cua đua xe while preserving the encoder-driven MVVM boundary.

**Architecture:** `EncoderDriveViewModel` remains the C++ home for turn-derived pose values. QML remains passive and binds vehicle yaw, lateral offset, and a new front-wheel steer property. The existing `HypercarView.qml` file is restyled in place to avoid a large rename/CMake churn.

**Tech Stack:** Qt 6/QML Shapes, C++17, Qt Test, CMake.

## Global Constraints

- Zero imperative JavaScript in QML.
- Modern C++17 only.
- MVVM: visual state comes from C++ `Q_PROPERTY` values.
- Keep the mock/hardware source-swap boundary unchanged.
- Do not stage `.superpowers/`.

---

### Task 1: C++ front-wheel steering cue

**Files:**
- Modify: `src/viewmodels/EncoderDriveViewModel.h`
- Modify: `src/viewmodels/EncoderDriveViewModel.cpp`
- Modify: `tests/tst_encoder_drive.cpp`

**Interfaces:**
- Consumes: `EncoderDriveViewModel::vehicleYawDegrees()`.
- Produces: `Q_PROPERTY(qreal frontWheelSteerDegrees READ frontWheelSteerDegrees NOTIFY frontWheelSteerDegreesChanged)`.

- [x] Add a failing Qt Test that expects strong turns to publish a finite front-wheel steering angle in the same sign direction as vehicle yaw, larger than gentle turn steering, and decaying toward zero on stale input.
- [x] Run `cmake --build build --target tst_encoder_drive -j2 && ./build/tests/tst_encoder_drive frontWheelSteerFollowsEncoderTurn`.
- [x] Add the C++ property, notify signal, member, update logic, and stale decay.
- [x] Rerun the focused test until it passes, then run all of `./build/tests/tst_encoder_drive`.

### Task 2: Race-kart vector restyle

**Files:**
- Modify: `qml/components/HypercarView.qml`
- Modify: `qml/components/EncoderDriveView.qml`
- Modify: `qml/Theme.qml`

**Interfaces:**
- Consumes: `EncoderDrive.vehicleLateralOffset`, `EncoderDrive.vehicleYawDegrees`, and `EncoderDrive.frontWheelSteerDegrees`.
- Produces: a passive vector race kart with open wheels, small chassis, steering front wheels, cockpit/seat detail, and road shadow.

- [x] Replace the hypercar silhouette with a go-kart/race-kart composition.
- [x] Bind front wheel item rotations directly to `frontWheelSteerDegrees`.
- [x] Keep all visual geometry declarative; no QML functions, control blocks, or mutable JavaScript.
- [x] Tune theme sizing tokens only if the kart reads too large/small.

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
- Consumes: implemented race-kart behavior.
- Produces: active Markdown that no longer describes the center vehicle as a hypercar-only visual.

- [x] Update active wording from hypercar to race kart / encoder kart where relevant.
- [x] Record that front-wheel steer is C++-owned and QML-bound.
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
- [x] Commit with `feat: replace hypercar with encoder race kart`.

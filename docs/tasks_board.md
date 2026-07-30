# 📋 Tasks Board (Vibe Coding Progress)

> **AI Context**: The source of truth for current project status. Mark tasks as `[x]` when verified.

## Phase 0: Project Scaffold (CMake & Arch)
- [x] CMakeLists.txt standardized to Qt 6.8+ (`qt_standard_project_setup`).
- [x] Directory structure established.
- [x] Documentation perfectly formatted for AI parsing.

## Phase 1: MVVM Core & Testing
- [x] Set up `tests/` directory and QtTest framework.
- [x] Implement `VehicleStatusViewModel` with TDD (tests first).
- [x] Ensure `main.cpp` uses `engine.loadFromModule()`.

## Phase 2: QML Shell & UI Guidelines
- [x] Define `Theme.qml` singleton with Cyberpunk design tokens.
- [x] Build QML Main Shell using pure property bindings (Zero JS).
- [x] Run `qt-qml-review` to verify compliance.

## Phase 3: Hardware Simulation (C++)
- [x] Build `SimulatorService` with `QTimer` telemetry generation.
- [x] Wire `SimulatorService` to `VehicleStatusViewModel`.

## Phase 4: STM32 Integration
- [x] Build `SerialService` parsing logic.
- [x] Ensure seamless swap with `SimulatorService` in C++.
- [x] Document final architecture layout.
- [ ] Field-validate the software pipeline with the target STM32, firmware, and USB-TTL hardware.

## Phase 5: Holographic Dashboard (Neon Cyberpunk 3-Panel)
- [x] Update C++ `VehicleStatusViewModel` (Battery, Range, Temp).
- [x] Update `SimulatorService` to generate mock data.
- [x] Implement `CircularGauge.qml` using `QtQuick.Shapes` (Approach A).
- [x] Implement `GlassPanel.qml` and `DashboardScreen.qml`.
- [x] Integrate into `Main.qml`.

## Phase 6: Advanced Mock Engine (Physics & Scenarios)
- [x] Create `MockScenarioEngine` class.
- [x] Implement realistic kinematics (Drag Race, Battery Drain).
- [x] Refactor `SimulatorService` to use `MockScenarioEngine`.
- [x] Update `CMakeLists.txt` and verify build.

## Phase 7: Dynamic Hardware Fallback
- [x] Expose `connectionStatusChanged` from `SerialService`.
- [x] Remove static `#define USE_SIMULATOR`.
- [x] Implement runtime switching logic in `main.cpp`.

## Phase 8: UI/UX Aesthetic Refinement
- [x] Refactor `CircularGauge` to `NeonTickGauge` with Tick-Based Illumination and Neon Bloom.
- [x] Implement precise "Double Arch" (Binocular) physical bezel silhouette using Cubic Bezier `PathSvg`.
- [x] Add secondary inset stroke for realistic angled glass edge reflection.
- [x] Upgrade central widget container to true Glassmorphism (translucent base + diagonal gradient).
- [x] Optimize unlit ticks and `DashboardScreen` layout margins for perfect alignment.
- [x] Condense and optimize AI Agent documentation skills (`.agents/workflows/`) for context efficiency.

## Phase 9: Music Player UI (Neon Cyberpunk)
- [x] Create `MusicPlayer.qml` component with basic layout and dummy `ListModel`.
- [x] Implement 3D Cover Flow using `PathView` with neon shadows and `OpacityMask`.
- [x] Build the Glassmorphism Navigation Bar (Controls, Progress Bar, Track Info).
- [x] Integrate `MusicPlayer.qml` into the central area of `DashboardScreen.qml`.
- [x] Fix CMake module linking by adding `MusicPlayer.qml` to `QML_FILES`.
- [x] Fix `DashboardScreen` layout bugs (replaced `RowLayout` with absolute coordinates to perfectly align gauges inside Double Arch bezels).
- [x] Fix SVG syntax typo in `Main.qml` Double Arch path.
- [x] Architect and implement `MusicScanner` C++ worker using `QThread` for non-blocking OS directory scanning.
- [x] Implement `MusicPlayerViewModel` (MVVM) inheriting `QAbstractListModel` with `QMediaPlayer` integration.
- [x] Bind `MusicPlayer.qml` directly to `MusicViewModel` exposing `progress`, `isPlaying`, and playback slots.

## Phase 10: Architecture Audit & Technical Debt Eradication
- [x] Run comprehensive Architecture Audit based on `AGENTS.md` and `Zero-JS` rule.
- [x] Fix Critical Multi-Threading Bugs (MusicPlayer UB, Rescan Leak).
- [x] Fix SerialService Watchdog Reconnect.
- [x] Run the Phase 10 Zero-JS cleanup in `NeonTickGauge.qml` and `MusicPlayer.qml`; its then-current scan did not cover every `Math` helper or handler/control-flow variant, so repository-wide completion is recorded in Phase 17.
- [x] Identify the SerialService MVVM leak; the actual extraction of serial derivation into `TelemetryMapper` and `main.cpp` was completed during Phase 17.
- [x] Implement proper SerialService write-back (`STOP`) and Checksum protocol validation.
- [x] Clean up the orphaned `CircularGauge.qml` and obsolete `QML_ELEMENT` usage. `GlassPanel.qml` and `MockScenarioEngine::setScenario` remain implemented, active project interfaces and were not deleted.
- [x] Add Unit Tests for `VehicleStatusViewModel::updateTelemetry` (corrected from stale `updateRawTelemetry` reference — added dedicated READ/WRITE/NOTIFY tests for `battery`, `range`, `temperature` in `tests/main.cpp`, complementing the existing `testUpdateTelemetry`).

## Phase 11: UI Standardization & Layout Refactor
- [x] Consolidate Design Tokens in `Theme.qml` (radii, durations, typography, geometry).
- [x] Componentize repetitive UI elements (`EnergyBlocks`, `GlassPanel`, `NeonIcon`, `NeonIconButton`).
- [x] Refactor most `MusicPlayer.qml` behavior to declarative bindings; the remaining scrubber state/math was moved to C++ during Phase 17 to complete the scrubber's Zero-JS migration.
- [x] Transition `DashboardScreen` and `NeonTickGauge` from absolute (magic number) coordinates to declarative `anchors` aligned precisely with the `PathSvg` Double Arch bezel.
- [x] Resolve all compiler warnings (`-Wconversion`) with proper `static_cast` in C++ ViewModels.
- [x] Fix `MusicPlayer` control panel padding — bottom `GlassPanel` was too short for its content, crushing the Play/Pause button against the bottom edge (140px → 176px, spacing bumped to `Theme.spaceMd`).

## Phase 12: Functional Telltale Bar
- [x] Add stroke-style SVG telltale icons (`warning-triangle`, `battery-low`, `temperature-high`) matching the existing icon set, registered in CMake `RESOURCES`.
- [x] Replace the 4 decorative mock `Rectangle`s in `DashboardScreen` top bar with 3 data-bound `NeonIcon` telltales: Warning (`isWarning`), Low Battery (`battery < 20`), High Temp (`temperature > 85`).
- [x] Add `Behavior on opacity` fade to `NeonIcon` so telltale lit/dim transitions animate per UI guidelines (Zero JS — pure declarative ternary bindings, no new backend properties).
- [x] Verify: full build clean, `ctest` 2/2 pass, 8s smoke run with no QML errors.

## Phase 13: Day/Night Theme + Startup Animation
- [x] Implement `ThemeViewModel` (TDD): `isNight` toggle + boot timeline (`bootStage`, `bootProgress`) driven by `QSequentialAnimationGroup`.
- [x] Register `ThemeController` context property and boot kick-off in `main.cpp`.
- [x] Theme.qml: Day (Light Glassmorphism) token set via ternaries + centralized `Behavior { ColorAnimation }`; constant `bezelStroke` keeps the Double Arch identical in both themes.
- [x] Sun/moon `NeonIconButton` toggle in the top bar (new stroke-style SVG icons).
- [x] Boot choreography: telltale self-test, gauge sweep 0→max→0, sequential center/bottom fade-in — all declarative ternary bindings (Zero JS).
- [x] Verify: clean build (0 warnings), all ctest pass (16 viewmodel tests), 8s smoke run clean.

## Phase 14: Vehicle Morphing (Bike / Scooter / Car)
- [x] Implement `VehicleModeViewModel` (TDD): `vehicleMode` string Q_PROPERTY + `cycleVehicleMode()` (car → bike → scooter → car), exposed as `VehicleMode` context property.
- [x] Add vehicle cycle `NeonIconButton` to the top bar (3 new stroke-style SVG icons: car/scooter/bike), disabled during boot.
- [x] Create `RangeTripCard.qml` (GlassPanel + EnergyBlocks) as the scooter center content, loaded via self-unloading async `Loader` (Zero-JS `active` binding).
- [x] Introduce QML `States`/`Transitions` in `DashboardScreen`: root `state` bound to `VehicleMode.vehicleMode`; "dip" transition (fade+scale both arches → `PropertyAction` swap → OutBack rise) masks Repeater tick relabeling.
- [x] Scooter: speed max 120, right gauge rebound to `vm.battery` (0–100, no redline); Bike: speed max 60, right arch shows large battery %, music player + bottom bar hidden.
- [x] Fix latent `EnergyBlocks` zero implicit size (invisible inside `Column`).
- [x] Historical Phase 14 verification: its then-current Zero-JS grep reported clean but did not cover every `Math` helper or handler/control-flow variant; build, 19 viewmodel tests, 8s smoke, and per-mode screenshots were recorded. Repository-wide Zero-JS completion is recorded in Phase 17.

## Phase 15: Drive Modes + Trip Computer
- [x] Sync `AGENTS.md` decision log with Phase 13/14 architecture (One-VM-per-Concern, Centralized Theme Ternaries, Boot Choreography, Dip Transition, MultiEffect Sibling Source rule).
- [x] Implement `DriveModeViewModel` (TDD): `driveMode`/`driveModeLabel` + `cycleDriveMode()` (normal → sport → eco → normal), context property `DriveMode`; 3 new stroke-style icons (leaf/dial/lightning).
- [x] Implement `TripComputerViewModel` (TDD): odometer/trip/avgSpeed integrated from speed × injected `elapsedMs` (wall-clock-free tests), `maxDeltaMs` clamp for stale gaps, `resetTrip()` keeps odometer; C++-formatted `tripDisplay`/`odoDisplay` strings (Zero-JS — no `.toFixed()` in QML).
- [x] Wire `QElapsedTimer` in `main.cpp` telemetry lambdas (`tripClock.restart()` returns elapsed ms); clock restart on telemetry source switch.
- [x] Theme.qml: `accentCyan` extended to 6 variants (ECO green / NORMAL cyan / SPORT orange × night/day) — existing singleton `Behavior` cross-fades the whole cluster; SPORT is orange, not red, to stay distinguishable from `warningRed`.
- [x] DashboardScreen: third top-bar cycle button (icon = next mode), `gearSubText` bound to `DriveMode.driveModeLabel` (scooter "BATT %" override preserved), bottom-center `TRIP x.x km · ODO y km` line with click-to-reset MouseArea.
- [x] Historical Phase 15 verification: its then-current Zero-JS grep reported clean but did not cover every `Math` helper or handler/control-flow variant; build, 28 viewmodel tests, 8s smoke, and drive-mode/theme screenshots were recorded. Repository-wide Zero-JS completion is recorded in Phase 17.

## Phase 17: Pre-Feature Baseline Repair
- [x] Extract newline framing and checksum validation into `SerialTelemetryParser`, including partial-frame retention and the 4096-byte buffer boundary.
- [x] Move raw serial-to-dashboard derivation into transport-independent `TelemetryMapper`, wired in `main.cpp`.
- [x] Make initial-open, valid-frame, stop, resource-error, watchdog, parser-reset, and reconnect transitions deterministic in `SerialService`.
- [x] Move music scrubber drag state, normalization, clamping, and seek requests into `MusicPlayerViewModel`; QML now forwards direct invokable calls.
- [x] Move volume normalization into C++, replace the remaining QML `Math` helpers with ternary bindings, and pass the exact repository-wide Zero-JS scan plus formal changed-line QML re-review (`58dae97`).
- [x] Register and pass the three focused targets reported by Tasks 1–3: `tst_viewmodels`, `tst_music_playback`, and `tst_serial_pipeline`.
- [x] Synchronize active project documentation with the committed implementation and preserve corrected historical context.
- [x] Complete the full pre-feature verification matrix and record its fresh evidence (Task 5).
- [x] Close the formal Task 5 review with in-scope C++ lint cleanup, direct Qt 6.11.1 `qmllint` triage, and a fresh full verification matrix.
- [x] Close final branch-review follow-ups: direct connected-state fallback/reconnect regressions, test-safe music persistence, and documentation/review-skill truth fixes (`4307a4d`, `1d863df`).
- [x] Complete the final re-review with no remaining high-confidence Critical or Important findings; rerun configure, build, CTest 3/3, Zero-JS, module `qmllint`, range diff hygiene, and the 8-second offscreen smoke check.
- [x] Historical checklist superseded by the canonical Phase 4 hardware-validation task.

## Phase 18: Rear Parking Assist
- [x] Add `MockParkingSensorService` for one deterministic rear ultrasonic distance and reverse-state sample.
- [x] Add `ParkingAssistViewModel` with `1..250` cm validation, Clear/Caution/Stop/Unavailable levels, formatted display state, and one-second stale expiry.
- [x] Add `CenterHubViewModel` so reverse automatically selects Parking Assist while preserving MusicPlayer lifetime.
- [x] Add passive Neon Cyberpunk/OEM `ParkingAssistView` with centralized theme tokens and no camera illustration.
- [x] Register `tst_parking_assist` and verify threshold, stale, mock progression, notification, and page-handoff contracts.
- [x] Document the future STM32 high-level distance/reverse boundary without changing the current UART protocol.

## Phase 19: Parking Assist UI + Bezel Clearance
- [x] Expose C++-derived `proximityProgress` (`0.0..1.0`) and `proximitySegments` (`0..8`) with focused notification coverage (`9bc7b78`).
- [x] Replace the static parking bars with a passive OEM hierarchy: sensor health, large distance, abstract centre obstacle/buffer zone, segmented track, and STOP-only pulse (`06e6295`).
- [x] Record the widened Double Arch/positive CenterHub-gap experiment (`898855f`); it is superseded by the compact baseline restoration below.
- [x] Restore the original compact bezel, cluster positions, and CenterHub overlap margins after visual review.
- [x] Close the 2026-07-30 uncommitted bezel-trial loop: visual review rejected every variant and the runtime was restored to the `f561572` compact Double Arch baseline; no trial geometry is retained.
- [x] Synchronize active architecture, UI/UX, routing, and journal documentation with the committed implementation.
- [x] Run and record the final cross-feature verification matrix: configure/build pass, CTest 4/4, Zero-JS comment-only matches, repository QML lint pass, module `qmllint` pass, and offscreen smoke reached the expected timeout with no QML/runtime errors.

## Phase 20: STOP-Gated Parking Assist Polish
- [x] Keep Music visible for clear/caution and auto-select Parking Assist only for live samples below `30` cm.
- [x] Remove the automatic `<=30` cm STOP sample from `MockParkingSensorService`; retain the high-level sample seam for manual/mock overrides and future STM32 input.
- [x] Add C++-owned live/stale/unavailable health, hysteresis, critical-proximity state, and animated-distance formatting.
- [x] Bind the passive QML view to the health state and numeric animation without JavaScript or mock-specific logic.
- [x] Verify the full configure/build/CTest/Zero-JS/QML-lint/module-qmllint/offscreen matrix and synchronize active documentation.

## Phase 21: CenterHub Manual Swipe
- [x] Add C++-owned page selection and an 80 px horizontal swipe threshold between Music and Distance Warning.
- [x] Add a null-target QML `DragHandler` that forwards pointer lifecycle/translation without JavaScript or window movement.
- [x] Keep the warning page manually inspectable for clear/caution/stale/unavailable states while preserving live `<30` cm automatic selection.
- [x] Enforce critical safety precedence and invalid-page rejection in `CenterHubViewModel` tests.
- [x] Run the complete configure/build/CTest/Zero-JS/QML-lint/module-qmllint/offscreen matrix and record fresh evidence before commit.

## Phase 22: Qt Creator VDPAU Runtime Investigation
- [x] Capture the Qt Creator-only startup failure and separate the optional VDPAU warning from the successful compiler/build result.
- [x] Keep the mitigation process-local in `main.cpp`; do not change host GPU drivers, environment-wide settings, or hardware.
- [x] Record the research evidence and the remaining Qt Creator rerun step in the runtime report.

## Phase 23: Lazy Multimedia Backend Startup
- [x] Defer `QMediaPlayer` construction until the first playback request while retaining the existing C++ audio contract.
- [x] Preserve headless test behavior and the MusicPlayer QML object graph.
- [x] Verify that offscreen startup no longer emits the VDPAU message; host PulseAudio warnings remain environmental.

## Phase 24: CenterHub Pointer Handler Compile Fix
- [x] Remove `anchors.fill` from the non-visual `DragHandler` that caused `Invalid property name "anchors"`.
- [x] Keep the C++ gesture forwarding contract and window drag handler unchanged.
- [x] Rebuild the Desktop Debug target and record the fix independently from the multimedia investigation.

## Phase 25: Code Review and Section Close
- [x] Remove the invalid `anchors` assignment from `CenterHub.qml`'s non-visual `DragHandler`.
- [x] Add and pass a regression for critical STOP → reverse-off page recovery in `tst_parking_assist`.
- [x] Run the C++ and QML review workflows; confirm no high-confidence actionable issue in the reviewed change set and no executable JavaScript in QML.
- [x] Re-run configure, full build, CTest 4/4, Desktop Debug build, module `qmllint`, offscreen smoke, and `git diff --check`.
- [x] Record the section-close evidence and keep the mock-data/hardware boundary explicit in the journal.
- [ ] Follow-up: make repeated `SerialService::startService()` idempotent before reopening an active port.
- [ ] Follow-up: validate positive `maxDeltaMs`, `staleIntervalMs`, and animation durations at C++ API boundaries.
- [ ] Follow-up: add bounded ID3/APIC frame-size and remaining-file checks in `MusicScanner`.

## Phase 26: Cyber Safety Mock Lab
- [x] Add `MockSafetyScenarioService` with a deterministic 72-second Normal → Advisory → Critical → Recovery → Complete mock script, bounded GUI-thread timer, direct `advance(qint64)` test seam, acknowledgement window, terminal lifecycle, and replay/reset behavior.
- [x] Add `SafetyScenarioViewModel` with effective-value notifications, mock-only copy/disclaimer, presentation-safe risk/threat/segment properties, acknowledgement/replay actions, and fail-safe availability handling.
- [x] Wire the stack-owned service and ViewModel as `SafetyScenario`; the one C++ gate allows it only in Car mode without critical parking proximity and never mutates UART, telemetry, or CenterHub state.
- [x] Add a passive Car overlay and top-bar launch control: the overlay remains a sibling above CenterHub, uses the abstract forward gate and eight declarative segments, and keeps the mock-only disclaimer visible without QML logic.
- [x] Make the lab an unambiguous center-panel takeover: fade the CenterHub render without destroying it, use concise C++-owned English copy (`SAFETY LAB` and `FORWARD HAZARD SIMULATION`), and reserve a non-overlapping `EXIT` action in the header.
- [x] Complete focused TDD and lifecycle regressions for the service and ViewModel, including deterministic clock boundaries, notifier suppression, acknowledgement, terminal completion, replay, service destruction, and availability-gate hiding.
- [x] Run C++ and QML deep reviews; no high-confidence actionable issue remained. `qmllint` was unavailable on this host and was not recorded as a passing check.
- [x] Run the fresh final matrix: configure and full build passed; CTest passed 5/5; the Zero-JS scan found only existing comment-only matches; project QML lint passed; the offscreen smoke ended with expected timeout `124` and only a host PulseAudio warning, with no QML/runtime error; and `git diff --check` passed.

## Phase 27: Cockpit Context Rail
- [x] Add a focused `CockpitContextViewModel` and `tst_cockpit_context` coverage for the effective compact context labels and notification contract.
- [x] Wire vehicle mode, drive mode, theme, Parking Assist, Safety Lab, and serial connection status without changing transport, mock, or Safety Lab contracts.
- [x] Render a passive, boot-gated five-pill context rail with token-only styling and collision-safe placement in Car, Bike, and Scooter modes.
- [x] Complete fresh configure/build/CTest verification with all six registered targets passing.

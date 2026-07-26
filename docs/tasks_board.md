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
- [x] Strip all imperative JS logic from `NeonTickGauge.qml` and `MusicPlayer.qml`.
- [x] Refactor SerialService MVVM Leak: move calculated telemetry back to `VehicleStatusViewModel`.
- [x] Implement proper SerialService write-back (`STOP`) and Checksum protocol validation.
- [x] Clean up orphaned files and dead code (`CircularGauge.qml`, `GlassPanel.qml`, `QML_ELEMENT`, `MockScenarioEngine::setScenario`).
- [ ] Add Unit Tests for `VehicleStatusViewModel::updateRawTelemetry` (STALE — no dedicated test file found in `tests/`; only `tst_music_playback.cpp` exists).

## Phase 11: UI Standardization & Layout Refactor
- [x] Consolidate Design Tokens in `Theme.qml` (radii, durations, typography, geometry).
- [x] Componentize repetitive UI elements (`EnergyBlocks`, `GlassPanel`, `NeonIcon`, `NeonIconButton`).
- [x] Refactor `MusicPlayer.qml` to use declarative bindings exclusively (removed Connections & JS imperative logic).
- [x] Transition `DashboardScreen` and `NeonTickGauge` from absolute (magic number) coordinates to declarative `anchors` aligned precisely with the `PathSvg` Double Arch bezel.
- [x] Resolve all compiler warnings (`-Wconversion`) with proper `static_cast` in C++ ViewModels.

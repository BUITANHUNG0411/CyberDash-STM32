# 🤖 AI Agent Master Router (AGENTS.md)

> **AI Context**: Root routing file for any AI agent or contributor working in this repository. Read it first to locate the domain rules and current project constraints.
> **Version**: 2026-07-27 | **Target**: Tool-neutral

## 1. Persona & Role
You are an **Elite Qt 6 / QML Expert and C++ Systems Engineer**. Your objective is to architect and implement the **QtStmAutomotiveSimulator**, a C++ Qt6 dashboard communicating with an STM32F103C8T6 via UART.

## 2. Global Directives (Zero Tolerance for Failure)
- **Zero JavaScript in QML**: NEVER write imperative JS in `.qml`. All logic MUST reside in C++.
- **Modern C++17**: Match `CMAKE_CXX_STANDARD 17`; enforce strict memory safety, clear ownership, and thread-safe operations.
- **MVVM Enforcement**: QML is a passive view. C++ exposes state via `Q_PROPERTY` and `Q_INVOKABLE`.

## 3. Context Routing (Read before coding)
Depending on the task, you MUST fetch and read the corresponding context:
- 🏗️ **Architecture & Data Flow**: Read `docs/architecture.md`
- 🎨 **UI / UX & QML Standards**: Read `docs/ui_ux_guidelines.md`. Use `resources/media/dashboard-preview.png` as the current canonical visual reference.
- 🔌 **Hardware & UART**: Read `docs/hardware_integration.md`
- 🧪 **Testing Strategies**: Read `docs/testing_strategy.md`
- 📋 **Current Task / Progress**: Read `docs/tasks_board.md`
- 📘 **C++ Coding Standards**: Read `.agents/workflows/cpp_coding_standards.md` when writing C++.
- 📙 **QML Coding Standards**: Read `.agents/workflows/qml_coding_standards.md` and `.agents/workflows/qml_docs_standards.md` when writing QML.
- 🛠️ **CMake Build System**: Read `.agents/workflows/cmake_standards.md` when configuring builds.

## 4. Workflows & Implicit Triggers
> **AI Context**: When facing specific task types, you MUST autonomously read and follow these workflow SOPs (Standard Operating Procedures).
- 🧠 **New Feature Planning**: Before writing any code for a new feature, architecture, or idea, you MUST implicitly read and execute `.agents/workflows/brainstorming.md`.
- ⚙️ **Standard Implementation (Vibe Coding)**: 
  1. Check `docs/tasks_board.md` for the current objective.
  2. Read the specific domain doc (e.g., UI or Testing).
  3. Implement tests first (TDD).
  4. Implement C++ logic -> Expose to QML -> Bind in QML.
  5. Verify build and tests before marking task as complete.

## 5. Project Overview & Architecture
> **AI Context**: High-level product requirements and architectural decisions for the QtStmAutomotiveSimulator.

### Product Vision
A scalable, highly interactive Qt 6 / QML PC application simulating a digital automotive dashboard. It morphs across Bike, Scooter, and Car modes with fluid animations, backed by a robust C++ engine that accepts UART telemetry.

**🎨 Design Inspiration**: We are building a "Neon Cyberpunk" UI shell. All frontend decisions must reflect this premium, dynamic, and futuristic aesthetic. Use `resources/media/dashboard-preview.png` as the canonical reference currently present in the repository.

### Technical Stack
- **Frontend**: Qt Quick (QML) - Declarative, zero logic.
- **Backend**: Modern C++17 (MVVM Pattern).
- **Build System**: CMake (Qt 6.8+ standards).
- **Hardware Integration**: STM32F103C8T6 via USB-TTL UART is implemented in software; physical-device field validation remains pending.

### Immutable Decision Log
> [!NOTE]
> Why we made these choices and why you should not change them.

| Decision | Rationale |
|---|---|
| **MVVM + Q_PROPERTY** | Best way to enforce "Zero JS in QML". C++ acts as ViewModel, QML acts as a pure reactive view. |
| **UART via QSerialPort** | Low-complexity, robust simulation of embedded systems suitable for hardware integration. |
| **State-Driven Layouts** | IMPLEMENTED (Phase 14): `DashboardScreen` root `state` bound to `VehicleMode.vehicleMode` (car/scooter/bike) — QML `States` + classic `PropertyChanges` (auto-restores original bindings on state exit). |
| **Runtime Hardware Fallback** | Dependency Injection in `main.cpp` attempts to open `SerialService` first. If hardware is disconnected, it instantly falls back to `SimulatorService` at runtime, ensuring UI continuity. |
| **Watchdog & Auto-Reconnect** | `SerialService` publishes a disconnected transition after stale input or a resource error. `main.cpp` activates `SimulatorService` fallback, while `SerialService`'s reconnect timer retries the port every 2000 ms. |
| **Double Arch Glass Frame** | Application runs as a Frameless Transparent Window (`Qt.FramelessWindowHint`). A `Shape` with a precise `PathSvg` acts as the custom hardware bezel, creating a "Double Arch / Binocular" physical dashboard silhouette. |
| **Glassmorphism Aesthetic** | Central panels use translucent backgrounds with subtle diagonal gradients (`#2C353F` tinted base) and fade-out masks to simulate frosted glass and cyberpunk neon lighting. |
| **Tick-Based Illumination** | Gauge tracking relies on dynamic illumination of discrete ticks (`isIlluminated`) rather than continuous solid arcs, maximizing the "Neon Cyberpunk" digital aesthetic without JS overhead. |
| **3D Cover Flow (PathView)** | Using native `PathView` with `PathAttribute` for performant, Zero-JS 3D music carousel instead of heavy 3D engines or complex JS math. |
| **Declarative Bezel Alignment** | `DashboardScreen` uses robust `anchors` and `Theme.qml` geometry tokens (e.g., `gaugeInsetLeft`) to lock gauge centers precisely to the `PathSvg` Double Arch bezels, maximizing maintainability while preventing UI overflow. |
| **Async Media Scanning** | Use `QThread` with `QDirIterator` (Worker Object pattern) in C++ to scan OS directories without blocking the QML Render Thread. |
| **C++ Audio Playback** | `QMediaPlayer` is managed entirely within `MusicPlayerViewModel`. Playback state and progress are exposed to QML via `Q_PROPERTY` to ensure Zero JS. |
| **One-ViewModel-per-Concern** | UI chrome state lives in dedicated small VMs (`ThemeViewModel`, `VehicleModeViewModel`, …) with their own context properties — independent lifecycles, focused TDD, never mixed into telemetry (`VehicleStatusViewModel`). |
| **Centralized Theme Ternaries** | `Theme.qml` color tokens are ternaries on chrome VMs (`ThemeController.isNight`, …) with `Behavior { ColorAnimation }` declared INSIDE the singleton — the whole app cross-fades with zero per-component changes. |
| **C++-Driven Boot Choreography** | Startup sequence (`bootStage`/`bootProgress`) is a `QSequentialAnimationGroup` timeline in `ThemeViewModel`; QML only binds ternaries (telltale self-test → gauge sweep → content fade-in). |
| **Dip Transition Masking** | Layout morphs use a sequential transition: fade+scale both arches to 0 → `PropertyAction` applies swaps while invisible → OutBack rise. Only `opacity`/`scale` are ever animated (never width/height on complex subtrees). |
| **MultiEffect Sibling Source** | NEVER capture an ancestor that contains the same `MultiEffect` (for example `source: parent`) because recursive capture freezes accumulated frames. Use a non-recursive sibling source; hide it only when it exists solely as the effect input. Visible gauge-tick and road-edge siblings are valid sources. |
| **Encoder-Compatible Perspective Road** | `PerspectiveRoadView` renders a fixed vehicle over one moving pseudo-3D road. All geometry comes from `RoadMotionViewModel`, currently fed by deterministic `MockWheelTelemetryService` left/right motion. Future hardware must supply measured encoder motion through the same boundary; commanded PWM is not odometry. |

## 6. Project Layout (Do not deviate without reason)
```text
src/viewmodels/   src/services/  (flat: SimulatorService, SerialService, SerialTelemetryParser, TelemetryMapper, MockScenarioEngine, MockWheelTelemetryService, MusicScanner)
qml/components/   qml/screens/   resources/   docs/   .agents/
```

## 7. Golden Checks Before Any "Done"
- [ ] Configure succeeds: `cmake -S . -B build`.
- [ ] Full build succeeds: `cmake --build build -j2`.
- [ ] All registered tests pass: `ctest --test-dir build --output-on-failure`.
- [ ] Zero-JS scan reports no executable JavaScript in `.qml`.
- [ ] Run the project `qt-qml-review` workflow for QML changes.
- [ ] New behavior has a C++ home (ViewModel/Service).
- [ ] QML remains unchanged when swapping Simulator ↔ Serial.
- [ ] Commit only after successful verification; push only when the repository state and user authorization permit it.

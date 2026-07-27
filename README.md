# QtStmAutomotiveSimulator

> **AI Context**: Repository overview for the C++17/Qt 6.8 Neon Cyberpunk dashboard, its verified software interfaces, deterministic tests, and pending hardware field validation.

A Qt Quick automotive dashboard with Car, Bike, and Scooter layouts, C++-owned state, music and map pages, and STM32F103C8T6 telemetry over UART. The application falls back to an in-process simulator whenever serial telemetry is unavailable.

![Dashboard preview](resources/media/dashboard-preview.png)

## Features

- Strict MVVM: seven C++ ViewModels are exposed to passive QML.
- Zero-JS policy: stateful interaction and domain logic live in C++; the repository-wide forbidden-pattern scan is clean at the Phase 17 implementation baseline.
- Car, Bike, and Scooter layouts driven by declarative QML states.
- Day/night themes and ECO/NORMAL/SPORT accent palettes.
- Tick-lit double-arch gauges, glass panels, telltales, boot choreography, and dip transitions.
- Car-mode CenterHub with persistent Music and Map pages.
- C++ `QMediaPlayer`, C++-owned scrubber state, and worker-thread music scanning.
- Implemented serial parser, telemetry mapper, watchdog, reconnect, and simulator fallback.
- Three deterministic Qt Test/CTest targets.

> [!NOTE]
> The serial software pipeline and no-hardware fallback transitions are automated and verified. Live STM32 firmware, USB-TTL wiring, unplug/replug behavior, motor control, and encoder feedback still need field validation.

## Architecture

`src/main.cpp` owns both services and all seven context ViewModels:

| QML context | Responsibility |
|---|---|
| `VehicleStatus` | Speed, RPM, gear, warning, battery, range, temperature |
| `MusicViewModel` | Library model, playback, progress, and scrubber |
| `ThemeController` | Day/night state and boot timeline |
| `VehicleMode` | Car/Bike/Scooter state |
| `DriveMode` | NORMAL/SPORT/ECO state |
| `TripComputer` | Odometer, trip, average speed, formatted displays |
| `MapModel` | Route progress derived from odometer distance |

The transport contracts are deliberately different:

```mermaid
flowchart LR
    Simulator[SimulatorService<br/>full dashboard telemetry] --> Gate[main.cpp source gate]
    Port[QSerialPort readyRead] --> Parser[SerialTelemetryParser]
    Parser --> Serial[SerialService<br/>raw RPM / VBat / error]
    Serial --> Mapper[TelemetryMapper]
    Mapper --> Gate
    Gate --> Vehicle[VehicleStatusViewModel]
    Gate --> Trip[TripComputerViewModel]
    Trip --> Map[MapViewModel]
    Vehicle --> QML[QML view]
```

`SimulatorService::telemetryUpdated(...)` supplies all dashboard fields. `SerialService::rawTelemetryUpdated(...)` supplies only parsed wire values. `TelemetryMapper::fromSerial(...)` derives dashboard fields outside the transport, and `main.cpp` updates the shared ViewModel surface. QML never selects a telemetry source.

Serial parsing runs in the GUI thread from `QSerialPort::readyRead` and performs no blocking waits. Only `MusicScanner` uses a worker thread for `QDirIterator`.

See [architecture.md](docs/architecture.md) for ownership, threading, MVVM, and Zero-JavaScript rules.

## Project Layout

```text
CyberDash-STM32/
├── AGENTS.md
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── services/
│   │   ├── MockScenarioEngine.{h,cpp}
│   │   ├── MusicScanner.{h,cpp}
│   │   ├── SerialService.{h,cpp}
│   │   ├── SerialTelemetryParser.{h,cpp}
│   │   ├── SimulatorService.{h,cpp}
│   │   └── TelemetryMapper.{h,cpp}
│   └── viewmodels/
│       ├── DriveModeViewModel.{h,cpp}
│       ├── MapViewModel.{h,cpp}
│       ├── MusicPlayerViewModel.{h,cpp}
│       ├── ThemeViewModel.{h,cpp}
│       ├── TripComputerViewModel.{h,cpp}
│       ├── VehicleModeViewModel.{h,cpp}
│       └── VehicleStatusViewModel.{h,cpp}
├── qml/
│   ├── Main.qml
│   ├── Theme.qml
│   ├── components/
│   │   ├── CenterHub.qml
│   │   ├── EnergyBlocks.qml
│   │   ├── GlassPanel.qml
│   │   ├── GlowingText.qml
│   │   ├── MusicPlayer.qml
│   │   ├── NeonIcon.qml
│   │   ├── NeonIconButton.qml
│   │   ├── NeonMapView.qml
│   │   ├── NeonTickGauge.qml
│   │   └── RangeTripCard.qml
│   └── screens/
│       └── DashboardScreen.qml
├── resources/
│   ├── icons/
│   └── media/
│       ├── dashboard-preview.png
│       └── simulator-demo.mp4
├── tests/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── tst_music_playback.cpp
│   └── tst_serial_pipeline.cpp
├── docs/
│   ├── DOCUMENTATION_STANDARDS.md
│   ├── architecture.md
│   ├── hardware_integration.md
│   ├── journal.md
│   ├── music_player_design.md
│   ├── tasks_board.md
│   ├── testing_strategy.md
│   └── ui_ux_guidelines.md
└── .agents/
    ├── skills/
    └── workflows/
```

## Requirements

- Qt 6.8 or newer with Core, Gui, Qml, Quick, Test, SerialPort, and Multimedia.
- CMake 3.16 or newer.
- A C++17 compiler.
- Optional for field validation: STM32F103C8T6 and USB-TTL adapter.

## Build and Run

```bash
cmake -S . -B build
cmake --build build -j2
./build/QtStmAutomotiveSimulator
```

The host currently constructs `SerialService` for `/dev/ttyUSB0`. If the open fails, the initial disconnected state starts `SimulatorService`. A successful open remains logically disconnected until a valid telemetry frame arrives.

## Testing

CTest registers exactly three targets:

| Target | Actual responsibilities |
|---|---|
| `tst_viewmodels` | Vehicle telemetry properties; theme/boot; vehicle and drive modes; trip computer; map progress |
| `tst_music_playback` | Playback state controls and C++ scrubber clamping/drag state; CTest supplies `QT_QPA_PLATFORM=offscreen` |
| `tst_serial_pipeline` | Parser, checksum, buffering, mapper, and no-hardware connection transitions |

Run the deterministic baseline:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Scan QML for forbidden patterns, then manually classify every match. Comment-only matches are non-executable; any executable match fails the policy:

```bash
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
```

For QML changes, also run the repository `qt-qml-review` workflow. A display-independent smoke command is:

```bash
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
```

See [testing_strategy.md](docs/testing_strategy.md) for evidence and troubleshooting rules.

## UART Protocol

The implemented receive format is newline-delimited ASCII:

```text
TEL,<rpm>,<battery-voltage>,<error-code>;<checksum>\n
```

The checksum is:

```cpp
(rpm + int(vbat) + error) & 0xFF
```

Valid examples:

```text
TEL,118,11.8,0;129\n
TEL,3000,11.8,0;195\n
```

`SerialTelemetryParser` retains partial frames and clears the buffer if it grows beyond 4096 bytes. `SerialService` uses a 500 ms valid-frame watchdog and a 2000 ms reconnect timer. A resource error or timeout closes the port, clears parser state, publishes disconnected once, and starts the simulator; a valid frame after reconnect publishes connected and stops the simulator.

The implemented outbound emergency helper sends:

```text
STOP;\n
```

The current host does not implement a checksummed outbound `SET` protocol. See [hardware_integration.md](docs/hardware_integration.md) before changing the wire contract.

## UI System

The current canonical reference is the preview at the top of this README. `qml/Theme.qml` centralizes palette, geometry, typography, and duration tokens.

- Day/night theme state comes from `ThemeController`.
- ECO is green, NORMAL is cyan/teal, and SPORT is orange; warning red stays distinct.
- `DashboardScreen` supports Car, Bike, and Scooter states.
- The Car CenterHub contains Music and Map pages.
- Vehicle changes use the fade/scale dip transition.
- Every `MultiEffect` captures a sibling source; recursive `source: parent` is forbidden.

See [ui_ux_guidelines.md](docs/ui_ux_guidelines.md).

## Roadmap and Status

Historical detail is preserved in [tasks_board.md](docs/tasks_board.md).

| Phase | Scope | Status |
|---|---|---|
| 0 | Project scaffold | Complete |
| 1 | MVVM core and testing | Complete |
| 2 | QML shell and UI guidelines | Complete |
| 3 | Hardware simulation | Complete |
| 4 | STM32 software integration | Implemented; field validation pending |
| 5 | Holographic dashboard | Complete |
| 6 | Advanced mock engine | Complete |
| 7 | Dynamic hardware fallback | Complete |
| 8 | UI/UX aesthetic refinement | Complete |
| 9 | Music player UI | Complete |
| 10 | Architecture audit and debt cleanup | Complete with historical claims corrected |
| 11 | UI standardization and layout refactor | Complete |
| 12 | Functional telltale bar | Complete |
| 13 | Day/night theme and startup animation | Complete |
| 14 | Vehicle morphing | Complete |
| 15 | Drive modes and trip computer | Complete |
| 16 | CenterHub and neon map | Complete |
| 17 | Pre-Feature Baseline Repair | Implementation complete; full verification pending |

Phase 17 repaired the parser/mapper boundary, made serial fallback transitions deterministic, moved scrubber and volume normalization into C++, removed the remaining QML `Math` helpers, and synchronized active documentation. The complete pre-feature build/test/smoke verification matrix remains a separate pending step.

## Contribution Rules

- Use C++17 and keep QML passive.
- Write focused tests first for behavior changes.
- Preserve `SimulatorService`/`SerialService` source-swap invariance at the ViewModel boundary.
- Complete configure, build, CTest, Zero-JS scan, and QML review checks before marking work done.
- Commit only after verification; push only with an appropriate repository state and user authorization.

## License

No `LICENSE` file is currently committed. Treat the repository as all rights reserved until the owner adds one.

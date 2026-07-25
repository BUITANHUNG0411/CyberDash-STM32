# Mock-Only Architecture Design

**Date:** 2026-07-25  
**Project identity:** Keep `CyberDash-STM32` and `QtStmAutomotiveSimulator` unchanged.

## Goal

Convert the current runtime to a focused mock automotive dashboard while
preserving the repository and executable names. Remove the production serial
port path and fix the confirmed C++/QML defects that prevent the mock showcase
from meeting its MVVM, Zero-JavaScript, testing, and performance targets.

## Scope

### Included

- Remove `SerialService` sources from the repository and build.
- Remove the `Qt6::SerialPort` dependency and runtime hardware fallback logic.
- Make `SimulatorService` the only telemetry provider.
- Preserve the existing `VehicleStatusViewModel` interface consumed by QML.
- Harden mock telemetry and media scanning inputs.
- Correct music playback state, rescan behavior, and cross-thread song delivery.
- Remove imperative JavaScript from the music scrubber.
- Correct confirmed QML binding, sizing, effect-source, and lifecycle defects.
- Make automated tests independent of a physical display and audio device.
- Document the resulting source tree and the responsibility of each file.
- Update architecture and task documentation to describe the current mock-only
  runtime accurately.

### Excluded

- Renaming the repository, executable, QML module, organization, or application.
- Adding another hardware protocol or backend abstraction.
- Redesigning the visual theme.
- Replacing Qt Multimedia or the asynchronous music scanner.
- Reintroducing STM32 or UART support in the current runtime.

## Architecture

The telemetry data flow becomes:

```text
SimulatorService
    -> raw telemetry signal
VehicleStatusViewModel
    -> validated Q_PROPERTY state
QML dashboard
```

`main.cpp` owns one `VehicleStatusViewModel`, one `SimulatorService`, and one
`MusicPlayerViewModel`. It connects simulator telemetry to the vehicle
ViewModel and starts simulation unconditionally. No service selection,
reconnection, watchdog, port discovery, or hardware fallback remains.

The media flow remains asynchronous:

```text
MusicScanner (worker thread)
    -> registered SongData value
MusicPlayerViewModel (UI thread)
    -> QAbstractListModel roles and playback state
MusicPlayer.qml
```

All `QAbstractListModel` mutations remain confined to the ViewModel thread.
`SongData` is declared and registered as a Qt metatype before the cross-thread
connection is used.

## Component Changes

### Simulator and vehicle state

`SimulatorService` remains responsible only for periodically producing mock
telemetry. `MockScenarioEngine` owns deterministic scenario progression and
resets elapsed scenario time when changing scenario.

`VehicleStatusViewModel` becomes the validation boundary. It rejects non-finite
values and clamps values to documented dashboard domains:

- speed: `0..300 km/h`
- RPM: `0..10000`
- battery: `0..100 percent`
- range: non-negative
- temperature: `-50..200 degrees Celsius`

Invalid mock input must never propagate NaN or impossible values into QML.

### Music scanning and playback

The APIC parser validates frame sizes, read results, required bytes, MIME type,
and a fixed maximum embedded-image size before indexing or allocating. Invalid
art is ignored without aborting the library scan.

Rescanning stops playback, clears the media source, resets selection and all
track-derived properties, emits the corresponding change notifications, and
then resets the model.

`QMediaPlayer::playbackStateChanged` drives the exposed playback state. Tests
use a guiless Qt test entry point and must not require a real audio server.

### QML

QML remains a passive view:

- Scrubber handlers forward pointer input to one C++ `Q_INVOKABLE`; they do not
  assign QML state in JavaScript blocks.
- Track information receives a defined width, avoiding circular implicit-size
  negotiation.
- `EnergyBlocks` exposes content-derived implicit dimensions.
- `NeonTickGauge` uses implicit default dimensions.
- `MultiEffect` never captures an ancestor containing itself.
- Dynamic cover images expose a declarative error fallback.
- Internal derived properties are readonly where consumer overrides are not
  part of the API.
- Effects and per-tick animations are reduced where they create avoidable
  continuous render cost.

Global context properties may remain for this iteration because replacing them
with typed injection is an architectural enhancement rather than a requirement
for removing serial hardware. It is recorded as a next-step recommendation.

## Testing

Changes follow red-green-refactor:

1. Add a failing regression test for each C++ behavior.
2. Run the focused test and confirm the expected failure.
3. Implement the smallest production change.
4. Run the focused test and the complete suite.

Required coverage:

- scenario switching resets time-dependent behavior;
- telemetry values are finite and within dashboard domains;
- `SongData` can cross a queued Qt connection;
- malformed/truncated APIC data does not crash or allocate unbounded memory;
- rescan clears playback-derived state;
- play/pause changes update the public playback state;
- the project configures without Qt SerialPort.

QML is verified by the project QML review linter, QML cache compilation, and a
repository search proving that no forbidden imperative JavaScript remains.

## Documentation and Tree

Add a repository structure document that lists every maintained source,
component, test, resource, and major documentation folder with its
responsibility. Historical hardware documentation may remain, but it must be
labelled as inactive reference material and must not describe the current
runtime as hardware-capable.

`docs/architecture.md` and `docs/tasks_board.md` will describe the mock-only
runtime and its verified completion state.

## Completion Criteria

- No `SerialService` source or `Qt6::SerialPort` build dependency remains.
- The application always starts `SimulatorService`.
- No imperative JavaScript control flow or multi-statement mutation remains in
  QML.
- All tests pass in the headless development environment.
- CMake configure and build complete successfully.
- Qt C++ and QML review lint results contain no unresolved critical findings in
  the changed scope.
- Documentation matches the implemented tree and runtime.
- Changes are committed and pushed without overwriting unrelated user work.

## Future Directions

Recommended follow-up work, outside this implementation:

1. Replace global context properties with typed ViewModel injection or
   registered singleton APIs.
2. Split the large `MusicPlayer.qml` into focused passive components.
3. Replace per-tick gauge delegates/effects with a batched Shape or custom
   renderer after profiling.
4. Add deterministic scenario controls, recorded telemetry playback, and
   scenario presets.
5. Add screenshot-based visual regression tests and QML profiler budgets.
6. Add library pagination, thumbnail caching, and cancellable/batched scanning.

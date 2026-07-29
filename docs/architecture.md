# 🏗️ Architecture: MVVM, Services, and Zero JavaScript

> **AI Context**: Active specification of ownership and data flow between the C++17 services, ten context ViewModels, and passive QML view.

## 1. Ownership and Lifetime

`main.cpp` owns all application services and the ten ViewModels exposed through the `QQmlContext`. They remain alive for the duration of `app.exec()`.

| Context property | C++ owner/type | Concern |
|---|---|---|
| `VehicleStatus` | `VehicleStatusViewModel` | Dashboard telemetry presented to QML |
| `MusicViewModel` | `MusicPlayerViewModel` | Library model, playback, and scrubber interaction |
| `ThemeController` | `ThemeViewModel` | Day/night theme and boot choreography |
| `VehicleMode` | `VehicleModeViewModel` | Car, Bike, and Scooter state |
| `DriveMode` | `DriveModeViewModel` | NORMAL, SPORT, and ECO state |
| `TripComputer` | `TripComputerViewModel` | Odometer, trip, and average speed |
| `ParkingAssist` | `ParkingAssistViewModel` | One rear ultrasonic distance, reverse state, hysteresis-stabilized proximity level, live/stale/unavailable health, formatted display, and presentation-safe progress |
| `CenterHubController` | `CenterHubViewModel` | C++-owned Music/Parking Assist page selection |
| `SafetyScenario` | `SafetyScenarioViewModel` | Mock-only forward-hazard lab presentation and user actions |
| `CockpitContext` | `CockpitContextViewModel` | Glanceable vehicle, drive, theme, telemetry-source, and Safety Lab context |

`SimulatorService`, `SerialService`, `MockSafetyScenarioService`, and `QElapsedTimer` are also stack-owned in `main.cpp`. QObject parent ownership covers each service's internal timers and serial port.

`main.cpp` sets `QT_FFMPEG_DECODING_HW_DEVICE_TYPES=,` before creating the Qt application.
This disables optional FFmpeg video hardware probing (including VDPAU) while keeping
`QMediaPlayer` audio playback available. `MusicPlayerViewModel` creates its `QAudioOutput`
at construction so the QML contract remains stable, but defers `QMediaPlayer` construction
until the first real play request; this keeps the optional FFmpeg probe out of dashboard
startup. It is an application-local compatibility fallback and does not install, remove, or
reconfigure host GPU drivers.

## 2. Telemetry Contracts

The two transports intentionally do not expose identical signals:

- `SimulatorService::telemetryUpdated(...)` emits a complete dashboard record: speed, RPM, gear, warning, battery, range, and temperature.
- `SerialService::rawTelemetryUpdated(...)` emits only parsed wire fields: RPM, battery voltage, and error code.
- `TelemetryMapper::fromSerial(...)` is the transport-independent conversion boundary. `main.cpp` maps raw serial data, then updates `VehicleStatusViewModel`.

```mermaid
flowchart LR
    Simulator[SimulatorService<br/>full dashboard telemetry] --> Main[main.cpp source gate]
    Port[QSerialPort readyRead] --> Parser[SerialTelemetryParser]
    Parser -->|raw RPM / VBat / error| Serial[SerialService]
    Serial --> Mapper[TelemetryMapper]
    Mapper --> Main
    Main --> Status[VehicleStatusViewModel]
    Main --> Trip[TripComputerViewModel]
    ParkingSource[MockParkingSensorService] -->|distance cm + reverse state| Parking[ParkingAssistViewModel]
    Parking --> Hub[CenterHubViewModel]
    VehicleMode[VehicleModeViewModel] --> SafetyGate[main.cpp presentation gate]
    Parking --> SafetyGate
    SafetyGate -->|setPresentationAllowed| Safety[SafetyScenarioViewModel]
    SafetySource[MockSafetyScenarioService<br/>deterministic mock timeline] --> Safety
    VehicleMode --> Cockpit[CockpitContextViewModel]
    DriveMode[DriveModeViewModel] --> Cockpit
    Theme[ThemeViewModel] --> Cockpit
    Serial -->|connection status| Cockpit
    Parking --> Cockpit
    Safety --> Cockpit
    Status --> DashboardQML[Passive dashboard QML]
    Trip --> DashboardQML
    Hub --> DashboardQML
    Parking --> DashboardQML
    Safety --> DashboardQML
    Cockpit --> DashboardQML
```

The `isHardwareConnected` gate ensures simulator updates are accepted only while serial is disconnected and serial updates only after a valid frame establishes the connection. A source switch restarts the trip clock so disconnected time is not integrated as distance.

The rear parking pipeline is independent of dashboard telemetry. `MockParkingSensorService`
emits one deterministic distance sample in centimetres together with reverse state;
`ParkingAssistViewModel` validates `1..250` cm, derives hysteresis-stabilized Clear/Caution/Stop/
Unavailable levels, exposes live/stale/unavailable sensor health, and expires valid input after
one second while reverse remains active. It also derives the
presentation-only properties `proximityProgress` (`0.0..1.0`: far/unavailable to stop) and
`proximitySegments` (`0..8`: unavailable to closest), avoiding distance math in QML.
`CenterHubViewModel` observes `criticalProximity` and selects Music page `0` or Parking Assist
page `1`; clear, caution, stale, and unavailable samples leave Music selected unless the user
manually drags the center hub. `CenterHub.qml` keeps a stable `StackLayout` and forwards its
horizontal `DragHandler` lifecycle to C++; an 80 px left/right threshold commits the two-page
selection. A live critical sample always overrides a manual Music request. Both ViewModels and
the mock timer remain GUI-thread objects. A future STM32 adapter
must convert ultrasonic echo timing into this high-level sample before it reaches the ViewModel;
QML never parses UART or hardware timing.

The Cyber Safety Lab is a separate mock-only pipeline. `MockSafetyScenarioService` owns the
deterministic Normal → Advisory → Critical → Recovery → Complete script, while
`SafetyScenarioViewModel` maps it to presentation-safe properties, copy, acknowledgement policy,
and the exact disclaimer `DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL`. Before QML loads,
`main.cpp` applies one availability gate: presentation is allowed only when
`VehicleMode.vehicleMode() == "car"` and `!ParkingAssist.criticalProximity()`. Losing either
condition stops an active lab through the Safety Scenario ViewModel; the gate does not write any
existing model, telemetry value, UART state, or CenterHub page. `SafetyScenarioOverlay.qml` is a
passive sibling above `CenterHub`, not a page or vehicle-layout state, so the lab neither represents
real sensing nor controls the vehicle. Its header reserves the C++-supplied title and instruction
on the left and the direct `EXIT` invokable action on the right, avoiding an overlapping
presentation command.

`CockpitContextViewModel` is a GUI-thread-only presentation adapter. It derives five compact
effective labels from existing vehicle mode, drive mode, theme, serial connection, Parking Assist,
and Safety Lab state; it never writes telemetry, UART state, CenterHub selection, Parking Assist,
or Safety Lab state. `CockpitContextRail.qml` only renders those labels and is not an interaction
surface.

## 3. Parser and Mapper Boundaries

`SerialTelemetryParser` owns byte buffering, newline framing, field conversion, and checksum validation. It accepts only a complete `TEL` line and clears an oversized buffer after it grows beyond 4096 bytes. In protocol notation, its checksum is exactly:

```cpp
(rpm + int(vbat) + error) & 0xFF
```

A valid example is:

```text
TEL,118,11.8,0;129\n
```

`TelemetryMapper` does not read bytes or manage connection state. It deterministically derives dashboard speed, gear, warning, battery, range, and temperature from a parsed raw record. This serial derivation no longer lives in `SerialService`.

## 4. Threading Model

`QSerialPort`, `SerialTelemetryParser`, `TelemetryMapper`, and ViewModel updates run on the GUI thread. `readyRead` work is bounded and non-blocking: read available bytes, split complete lines, validate, emit.

Only media directory scanning runs on a worker thread. `MusicScanner` uses the worker-object pattern with `QThread` and `QDirIterator`; results return to `MusicPlayerViewModel` through queued signals. Parking sensor, page-selection, and safety-scenario timers remain bounded GUI-thread work. The Safety Scenario service also exposes direct `advance(qint64)` progression so tests drive its deterministic clock without an event loop.

> [!WARNING]
> Do not move a QObject with a parent to another thread, block the GUI thread waiting for serial input, or access QML-owned state from the music scanner worker.

## 5. Zero-JavaScript Rule

QML is a passive declarative view. Imperative functions, control flow, local mutation, and block-form event handlers are forbidden. Property bindings, ternary presentation bindings, declarative `States`/`Transitions`, and one direct `Q_INVOKABLE` call from a handler are allowed.

```qml
MouseArea {
    onPressed: MusicViewModel.beginScrub(mouseX, width)
    onPositionChanged: MusicViewModel.updateScrub(mouseX, width)
    onReleased: MusicViewModel.endScrub()
}
```

Scrubber drag state and normalization are C++ properties and invokables; QML does not calculate or retain scrub state.

## 6. MVVM Standard

Every ViewModel inherits `QObject`, declares `Q_OBJECT`, exposes observable state through `Q_PROPERTY`, and emits its `NOTIFY` signal only when the effective value changes. New behavior must have a C++ home before QML binds to it.

## Troubleshooting

- **Simulator never starts:** confirm `SerialService::startService()` publishes the initial disconnected state and that the `connectionStatusChanged(false)` connection is installed first.
- **Valid serial bytes do not update the UI:** verify the newline and checksum, then confirm `rawTelemetryUpdated` reaches `TelemetryMapper` while `isHardwareConnected` is true.
- **Partial data survives a disconnect:** all stop, resource-error, and watchdog paths must call `SerialTelemetryParser::clear()`.
- **UI freezes during scanning:** confirm only `MusicScanner` performs `QDirIterator` work and that it remains on its worker thread.
- **QML contains interaction math:** move it into a ViewModel invokable/property and leave only a direct call or binding in QML.

# 🔌 Hardware Integration (STM32 and UART)

> **AI Context**: Active protocol and runtime-fallback contract for STM32F103C8T6 telemetry over `QSerialPort`. Software integration is implemented; physical-device field validation is still pending.

## 1. Hardware Topology

- **Host:** Qt 6 application using `QSerialPort`.
- **MCU:** STM32F103C8T6 (Blue Pill).
- **Connection:** USB-TTL UART at 115200 baud, 8 data bits, no parity, 1 stop bit, no flow control.
- **Default host path:** `/dev/ttyUSB0`, constructed in `src/main.cpp`.

## 2. Receive Protocol

Incoming telemetry is newline-delimited ASCII:

```text
TEL,<rpm>,<battery-voltage>,<error-code>;<checksum>\n
```

The checksum is the low eight bits of the integer sum:

```cpp
(rpm + int(vbat) + error) & 0xFF
```

The battery voltage is truncated toward zero for checksum calculation. A valid complete frame is:

```text
TEL,118,11.8,0;129\n
```

Here, `118 + 11 + 0` equals `129`. The parser rejects malformed fields, multiple separators, non-finite or out-of-`int` battery values, negative/mismatched checksums, and non-`TEL` records.

## 3. Framing and Mapping

`SerialTelemetryParser` appends bytes into a `QByteArray`, retains partial lines, and processes only newline-terminated records. If the retained buffer grows beyond 4096 bytes, it is cleared and that append produces no frame.

`SerialService` emits only:

```cpp
rawTelemetryUpdated(int rpm, double batteryVoltage, int errorCode)
```

`main.cpp` passes that raw record to `TelemetryMapper`. The mapper derives dashboard speed, gear, warning, battery, range, and temperature before updating `VehicleStatusViewModel`. Serial transport code does not own dashboard derivation.

## 4. Runtime Connection State

The service publishes disconnected before its initial open attempt. Opening a port is not sufficient to claim a connection; only the first valid `TEL` frame emits `connectionStatusChanged(true)`.

- **Watchdog:** 500 ms from the latest valid frame.
- **Reconnect timer:** 2000 ms after an open failure, watchdog timeout, or resource error.
- **Parser reset:** stop, watchdog, and resource-error paths clear partial input.
- **Reconnect:** a successful reopen restarts the watchdog but remains disconnected until valid telemetry arrives.

```text
QSerialPort::ResourceError
  -> stop/close port + clear parser
  -> connectionStatusChanged(false)
  -> main.cpp starts SimulatorService
  -> reconnect attempt every 2000 ms
  -> valid TEL frame
  -> connectionStatusChanged(true)
  -> main.cpp stops SimulatorService
```

Duplicate disconnected notifications are suppressed. This keeps source switching deterministic and prevents multiple fallback transitions for one failure.

## 5. Threading

`QSerialPort::readyRead`, byte parsing, checksum validation, mapping, and ViewModel updates run on the GUI thread. The handler performs no blocking wait and only consumes currently available bytes.

> [!WARNING]
> Do not add blocking reads or move `QSerialPort` independently of its owning `SerialService`. The only worker-thread I/O in the current application is music directory scanning.

## 6. Host-to-MCU Commands

`sendCommand()` appends a newline to the supplied text. The implemented emergency-stop helper sends:

```text
STOP;\n
```

No checksummed outbound `SET` protocol is implemented in the current host code; coordinate and test any firmware command extension before documenting it as supported.

## 7. Validation Status

Parser, mapper, and no-hardware connection transitions have deterministic automated coverage in `tst_serial_pipeline`. Live STM32 wiring, firmware compatibility, unplug/replug behavior, and motor control still require the separate Phase 4 field validation.

## 8. Future Position-Source Boundary

Phase 20 is mock-first: `MockPositionSource` emits deterministic `QGeoPositionInfo` samples to
`MapViewModel`. Future GNSS can replace that source through the same Qt position-source signal
boundary without changing QML.

Encoder measurements are not absolute map coordinates. A future encoder/dead-reckoning adapter
must include explicit localization or map matching before it emits a coordinate and bearing to
the map boundary. Commanded PWM is an actuator command, not motion feedback or odometry, and it
must never be presented as geographic position. PWM may remain diagnostic/command telemetry.

## Troubleshooting

- **Frame rejected:** include the terminating newline and recompute the checksum using the truncated integer part of battery voltage.
- **Port opens but simulator remains active:** this is expected until a valid telemetry frame arrives.
- **Repeated reconnects:** check device permissions, the `/dev/ttyUSB0` path, baud settings, firmware line endings, and checksum output.
- **Stale partial frame after unplug:** ensure every failure path reuses `stopService()`, which clears the parser.
- **UI stays on hardware after silence:** confirm the 500 ms watchdog is running after open and after every valid frame.
- **Future map position is implausible:** validate GNSS accuracy or the localization adapter before changing `MapViewModel` or QML.
- **Only PWM is available:** do not derive or label map position from it; retain the deterministic mock source until a qualified position source exists.

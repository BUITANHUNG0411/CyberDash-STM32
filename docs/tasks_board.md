# Tasks Board

> **AI Context**: This board describes only the current Car dashboard and its remaining work.
> Removed feature branches are intentionally not recorded here.

## Active product

- [x] Keep a single Car dashboard layout with no vehicle-mode switcher.
- [x] Keep telemetry, SimulatorService fallback, SerialService, and TelemetryMapper on the
  shared `VehicleStatusViewModel` boundary.
- [x] Keep day/night theme, Music, Trip Computer, and the two-page Music/Parking CenterHub.
- [x] Keep Parking Assist thresholding, hysteresis, health, formatting, and critical precedence in
  C++.
- [x] Enforce passive QML and zero executable JavaScript.
- [x] Register and pass the four focused CTest targets.

## Remaining work

- [ ] Field-validate the software pipeline with the target STM32, firmware, and USB-TTL hardware.
- [ ] Make repeated `SerialService::startService()` calls idempotent before reopening an active
  port.
- [ ] Validate positive `maxDeltaMs`, `staleIntervalMs`, and animation durations at C++ API
  boundaries.
- [ ] Add bounded ID3/APIC frame-size and remaining-file checks in `MusicScanner`.

## Completion checks

- [ ] `cmake -S . -B build`
- [ ] `cmake --build build -j2`
- [ ] `ctest --test-dir build --output-on-failure`
- [ ] Zero-JS scan and relevant QML/C++ review workflows
- [ ] Offscreen smoke launch when the host permits it

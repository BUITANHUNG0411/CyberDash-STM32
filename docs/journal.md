# Project Journal

> **AI Context**: This file contains only durable decisions for the current product. Removed
> presentation branches are not retained as historical feature entries.

## Current product contract

- The application exposes one stable Car dashboard.
- The active graph is limited to UART/simulator telemetry, day/night theme, Music, Parking
  Assist, Trip Computer, and the C++-owned CenterHub page selection between Music and Parking.
- QML is a passive view with zero executable JavaScript; behavior belongs in C++17 ViewModels and
  services.
- The simulator and SerialService share the same `VehicleStatusViewModel` boundary, with runtime
  fallback when hardware is unavailable.
- Parking Assist consumes one validated high-level `distanceCm + reverseActive` sample. It owns
  thresholding, hysteresis, health, formatting, and critical-page precedence.
- Physical STM32/USB-TTL field validation remains pending.

## Verification baseline

The repository baseline requires CMake configure, a full `-j2` build, all registered CTest
targets, the repository Zero-JS scan, QML/C++ review workflows when applicable, and an offscreen
smoke launch before completion claims.

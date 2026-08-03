# Trip Computer Visual Refresh Design

**Date:** 2026-08-03
**Status:** Implemented locally, pending human review
**Scope:** CenterHub Trip Computer presentation and Parking Assist distance scale

## Goal

Give the Trip Computer the same visual authority as the Parking Assist page while
reducing the Parking Assist distance readout so it fits the compact double-arch
dashboard without dominating the panel.

## Current Problem

Parking Assist uses the shared `Theme.displayMd` token for its distance value, making
`180 CM` visually oversized. Trip Computer presents three values in a flat column
with weak grouping, so the page reads as a plain information sheet rather than a
focused automotive instrument.

## Design

- Add dedicated `Theme.qml` typography tokens for the parking distance, Trip hero,
  Trip stat values, and the small Trip status line.
- Keep Parking Assist's existing data hierarchy and behavior, but bind its animated
  distance text to the smaller parking-specific token.
- Recompose Trip Computer as a dark glass data card:
  - compact header with `TRIP COMPUTER` and `DRIVE DATA` context;
  - one cyan-glow hero for `TripComputer.tripDisplay`;
  - two bordered stat cards for ODO and AVG SPEED;
  - a restrained session rail and explicit `RESET TRIP` affordance;
  - existing keyboard navigation hint retained as a low-priority footer.
- Reset remains a direct `TripComputer.resetTrip()` call from QML. No new state,
  calculation, or formatting is introduced in QML.

## Constraints

- QML remains declarative and contains zero executable JavaScript.
- All display strings continue to come from `TripComputerViewModel`.
- Reuse existing `GlassPanel`, `GlowingText`, theme colors, spacing, radii, and
  animation durations.
- Do not change CenterHub page ownership, parking safety precedence, telemetry,
  navigation, or application geometry.
- Do not add dependencies or commit changes.

## Acceptance Criteria

- Parking Assist distance uses a dedicated smaller token and no longer binds to
  `Theme.displayMd`.
- Trip Computer has a visually dominant trip value, two distinct secondary stat
  surfaces, a reset affordance, and retained navigation guidance.
- Trip Computer consumes only existing C++ properties and the existing reset
  invokable.
- The source-contract test asserts the key visual structure and zero-JavaScript
  policy.
- Configure, build, all CTest targets, the repository zero-JS scan, QML review, and
  an offscreen smoke launch complete without new errors.

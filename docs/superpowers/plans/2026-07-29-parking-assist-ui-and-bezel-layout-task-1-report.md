# Task 1 Report: Parking Assist Presentation Progress

## Scope

Added the presentation-only `ParkingAssistViewModel` properties required by the OEM distance panel:

- `proximitySegments`: `0` when unavailable; `1..8` for valid samples, increasing as the detected obstacle gets closer.
- `proximityProgress`: `0.0` when unavailable or at 250 cm; a clamped `0.0..1.0` scale from 250 cm to the existing 30 cm stop threshold.

The existing valid range (`1..250` cm), reverse handling, proximity levels, and stale-data behavior are unchanged.

## TDD evidence

### RED

1. Added `derivesPresentationSafeProximityProgress` to `tests/tst_parking_assist.cpp` before production changes. It asserts clear (250 cm), caution (150 cm), stop (30 cm), unavailable (0 cm), and duplicate-notification suppression.
2. Ran:

   `cmake --build build --target tst_parking_assist -j2`

3. Result: expected build failure. The test referenced the missing `proximitySegmentsChanged`, `proximityProgressChanged`, `proximitySegments`, and `proximityProgress` API members.

### GREEN

1. Added the two `Q_PROPERTY` declarations, accessors, and effective-value notification signals.
2. Derived progress as `(250 - distanceCm) / (250 - 30)`, clamped to `0.0..1.0`; derived segments as `1 + floor(progress * 7)`, clamped to `1..8` for valid data.
3. Ran:

   `cmake --build build --target tst_parking_assist -j2 && QT_QPA_PLATFORM=offscreen ./build/tests/tst_parking_assist`

4. Result: PASS — 10 passed, 0 failed, 0 skipped.

## Scope limits

No QML, theme, CMake, reverse/stale semantics, or ultrasonic input thresholds were changed. Full-project verification and UI linting are owned by the plan's final integration task.

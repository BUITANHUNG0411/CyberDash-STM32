# Task 3 Report: Widened Bezel and Center Clearance

## Diff Summary

- Added centralized bezel shoulder, arc-radius, reflection-inset, gauge-center,
  and center-gap geometry tokens to `qml/Theme.qml`.
- Bound both `PathSvg` layers in `qml/Main.qml` to the bezel tokens. The arch
  shoulders now extend outward while the outer arc boundary remains inside the
  existing frameless window.
- Moved both gauge centers outward through the existing Theme insets and
  replaced the CenterHub overlap margins with a positive tokenized clearance.
- Preserved the dashboard's anchored layout, depth ordering, and vehicle-mode
  state transitions. No C++ or parking-panel behavior changed.

## Verification

- `git diff --check`
- `python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml`
- `/usr/lib/qt6/bin/qmllint --module com.showcase -I build`
- `cmake --build build -j2`
- `QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator`

The QML linter and build completed successfully. The offscreen smoke process
remained running until the expected `timeout` stop; it emitted no QML/runtime
errors. The only observed runtime output was the sandboxed PulseAudio warning
(`pa_write() failed ... Operation not permitted`).

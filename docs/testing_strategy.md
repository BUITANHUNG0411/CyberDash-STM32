# 🧪 Testing Strategy

> **AI Context**: Active deterministic test and verification contract for C++17, Qt Test, QML policy checks, and headless execution.

## 1. TDD Loop

> [!IMPORTANT]
> For a new behavior or bug fix, first create a focused failing test, then make the smallest production change that passes it.

1. **Red:** add a test that fails for the missing or incorrect behavior.
2. **Green:** implement the C++ behavior and pass the focused target.
3. **Refactor:** improve structure without changing the tested contract.
4. **Bind:** expose the behavior through `Q_PROPERTY` or `Q_INVOKABLE` when QML needs it.
5. **Verify:** configure, build, run all CTest targets, scan QML, review QML changes, and smoke-test when the environment supports it.

## 2. Registered Test Targets

| CTest target | Responsibilities |
|---|---|
| `tst_viewmodels` | `VehicleStatusViewModel` property READ/WRITE/NOTIFY behavior; theme and boot state; vehicle/drive mode cycles; trip integration |
| `tst_music_playback` | Repeat/shuffle/volume/seek/playback state and C++-owned scrubber clamping/drag state; multimedia-disabled construction for deterministic tests |
| `tst_serial_pipeline` | Parser framing/checksum/buffer boundaries; raw-to-dashboard mapper; initial, valid-frame, stop, resource-error, and parser-reset connection transitions |
| `tst_parking_assist` | One-sensor distance thresholds, invalid/unavailable input, stale timeout, duplicate-notification suppression, mock progression, and CenterHub page handoff |

CTest configures `QT_QPA_PLATFORM=offscreen` and a 20-second timeout for `tst_music_playback`, so the multimedia-facing test is deterministic in a headless environment. The serial tests use controlled/no-hardware paths and do not require an attached STM32.

## 3. Coverage Requirements

- Every `Q_PROPERTY` must cover its READ accessor, WRITE mutator when present, and effective-value `NOTIFY` behavior.
- Parser tests must cover a valid frame, invalid checksum, partial input, oversized input, numeric boundaries, and parser reset.
- Mapper tests must prove dashboard derivation occurs outside `SerialService`.
- Connection tests must cover initial disconnected publication, valid-frame connection, idempotent resource errors, stop behavior, and partial-frame clearing.
- Music interaction tests must cover scrubber normalization, clamping, zero-width input, drag state, and signal emission.
- Parking tests must cover valid `1..250` cm thresholds, invalid values, reverse page selection, stale expiry after one second, effective-value notifications, and deterministic mock progression.
- QML interaction handlers remain direct invokable calls; no QML-local scrubber state or math.

## 4. Deterministic Verification Commands

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Run the repository-wide policy scan exactly, then manually classify every match. Comment-only matches are non-executable; any executable match fails the policy:

```bash
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
```

For QML changes, run the repository's `qt-qml-review` workflow after the scan. It provides the project-specific Zero-JavaScript and declarative-structure review.

Its deterministic first pass can also be run directly:

```bash
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
```

If a display-capable Qt runtime is available, smoke the application without treating the expected timeout as a crash:

```bash
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
```

## 5. Evidence Rules

Record the command, exit/result, and focused RED/GREEN evidence in the task report. Do not mark a phase-wide verification item complete from an old phase note or a selected test subset. Physical STM32 validation and real audio-device playback are separate field checks.

## Troubleshooting

- **`tst_music_playback` aborts headlessly:** run it through CTest so its `QT_QPA_PLATFORM=offscreen` property is applied.
- **A focused test passes but CTest fails:** rebuild all targets and run unfiltered CTest; selected targets do not establish full baseline health.
- **Serial test waits for hardware:** use the controlled open seam and injected bytes; unit tests must not depend on `/dev/ttyUSB0`.
- **Zero-JS scan reports matches:** move utility math, block handlers, functions, and mutable JavaScript state into C++, then rerun the scan and QML review.
- **Smoke command exits 124:** `timeout` uses 124 for a still-running application; inspect output for QML/runtime errors before deciding whether the smoke passed.

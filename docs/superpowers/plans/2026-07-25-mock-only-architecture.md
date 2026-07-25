# Mock-Only Architecture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the active serial/STM32 runtime and deliver a verified, mock-only Qt/QML showcase with corrected telemetry, media, QML, tests, and repository documentation.

**Architecture:** `SimulatorService` is the sole telemetry producer and feeds a validated `VehicleStatusViewModel`; QML remains a passive view. `MusicScanner` stays on a worker thread and sends a registered value type to `MusicPlayerViewModel`, which exclusively owns model and playback state on the UI thread.

**Tech Stack:** C++17, Qt 6.8+ Core/Gui/Qml/Quick/Multimedia/Test, QML, CMake, QtTest.

## Global Constraints

- Keep the repository name `CyberDash-STM32`.
- Keep the executable/project name `QtStmAutomotiveSimulator`.
- Zero imperative JavaScript in every `.qml` file.
- Preserve MVVM with behavior in C++ and state exposed through `Q_PROPERTY`, `NOTIFY`, and `Q_INVOKABLE`.
- Remove `SerialService` and `Qt6::SerialPort` from the active repository runtime.
- Follow red-green-refactor for every behavioral bugfix.
- Do not alter the Neon Cyberpunk visual direction.

---

### Task 1: Remove the serial runtime

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`
- Delete: `src/services/SerialService.h`
- Delete: `src/services/SerialService.cpp`
- Test: `tests/CMakeLists.txt`
- Test: `tests/tst_mock_architecture.cpp`

**Interfaces:**
- Consumes: `SimulatorService::telemetryUpdated(...)`.
- Produces: an application runtime with `SimulatorService` as the only telemetry source.

- [ ] **Step 1: Add a failing architecture regression test**

Create `tests/tst_mock_architecture.cpp` that reads the source CMake file and
asserts it contains neither `SerialService` nor `Qt6::SerialPort`, then add the
test target:

```cpp
void serialRuntime_isAbsent()
{
    QFile cmakeFile(PROJECT_SOURCE_DIR "/CMakeLists.txt");
    QVERIFY(cmakeFile.open(QIODevice::ReadOnly));
    const QByteArray contents = cmakeFile.readAll();
    QVERIFY(!contents.contains("SerialService"));
    QVERIFY(!contents.contains("Qt6::SerialPort"));
}
```

- [ ] **Step 2: Run the focused test and verify RED**

Run: `cmake -S . -B build && cmake --build build --target tst_mock_architecture -j2 && ctest --test-dir build -R tst_mock_architecture --output-on-failure`

Expected: FAIL because the current `CMakeLists.txt` contains `SerialService` and `Qt6::SerialPort`.

- [ ] **Step 3: Remove hardware sources and simplify startup**

Change `find_package`/`target_link_libraries` to omit SerialPort, delete the two
SerialService files, and reduce `main.cpp` to:

```cpp
VehicleStatusViewModel vehicleStatus;
SimulatorService simulatorService;
QObject::connect(&simulatorService,
                 &SimulatorService::telemetryUpdated,
                 &vehicleStatus,
                 &VehicleStatusViewModel::updateTelemetry);
simulatorService.startSimulation();
```

Keep the existing engine and music ViewModel setup.

- [ ] **Step 4: Run the focused test and build**

Run: `cmake -S . -B build && cmake --build build -j2 && ctest --test-dir build -R tst_mock_architecture --output-on-failure`

Expected: PASS and the application target links without Qt SerialPort.

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt src/main.cpp src/services tests
git commit -m "refactor: make simulator the sole telemetry backend"
```

### Task 2: Harden mock telemetry and scenario behavior

**Files:**
- Modify: `src/services/MockScenarioEngine.cpp`
- Modify: `src/services/SimulatorService.cpp`
- Modify: `src/viewmodels/VehicleStatusViewModel.cpp`
- Modify: `src/viewmodels/VehicleStatusViewModel.h`
- Modify: `tests/main.cpp`

**Interfaces:**
- Consumes: `VehicleStatusViewModel::updateTelemetry(double, int, const QString &, bool, int, int, int)`.
- Produces: finite, domain-bounded vehicle properties and deterministic scenario resets.

- [ ] **Step 1: Add failing tests**

Add QtTest cases proving that negative/non-finite/out-of-domain telemetry is
clamped to speed `0..300`, RPM `0..10000`, battery `0..100`, range `>=0`, and
temperature `-50..200`. Add a scenario test that advances one scenario,
switches away and back, and expects a fresh initial phase.

- [ ] **Step 2: Verify RED**

Run: `cmake --build build --target tst_viewmodels -j2 && ./build/tests/tst_viewmodels`

Expected: FAIL on out-of-domain properties and preserved scenario phase.

- [ ] **Step 3: Implement bounded state**

Use `std::isfinite` and `std::clamp` at the ViewModel boundary. Reset
`m_elapsedTimeMs` inside `MockScenarioEngine::setScenario`. Replace C-style
casts and warning-producing conversions with explicit `static_cast` after
clamping.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build build --target tst_viewmodels -j2 && ./build/tests/tst_viewmodels`

Expected: all vehicle and scenario tests PASS without conversion warnings in changed files.

- [ ] **Step 5: Commit**

```bash
git add src/services/MockScenarioEngine.cpp src/services/SimulatorService.cpp src/viewmodels/VehicleStatusViewModel.* tests/main.cpp
git commit -m "fix: validate mock telemetry domains"
```

### Task 3: Correct asynchronous scanning and playback state

**Files:**
- Modify: `src/services/MusicScanner.h`
- Modify: `src/services/MusicScanner.cpp`
- Modify: `src/viewmodels/MusicPlayerViewModel.h`
- Modify: `src/viewmodels/MusicPlayerViewModel.cpp`
- Modify: `tests/tst_music_playback.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: registered `SongData`, bounded cover parsing, coherent rescan state, and `Q_INVOKABLE void scrubTo(qreal ratio)`.

- [ ] **Step 1: Add failing regression tests**

Add tests for:

```cpp
QVERIFY(QMetaType::fromType<SongData>().isValid());
vm.setVolume(0.5F);
vm.scanLibrary();
QCOMPARE(vm.currentIndex(), -1);
QCOMPARE(vm.progress(), 0.0F);
QCOMPARE(vm.positionMs(), 0);
QCOMPARE(vm.duration(), 0);
```

Expose APIC parsing as a focused non-QObject helper or private implementation
unit so truncated frames and oversized image payloads can be tested without
filesystem/audio devices. Add a test that calls `scrubTo(-1.0)` and
`scrubTo(2.0)` and confirms safe ratio clamping.

- [ ] **Step 2: Verify RED**

Run: `cmake --build build --target tst_music_playback -j2 && QT_QPA_PLATFORM=offscreen ./build/tests/tst_music_playback`

Expected: FAIL for missing metatype, incomplete reset, unsafe parser helper, or missing scrub API.

- [ ] **Step 3: Implement the fixes**

Declare `Q_DECLARE_METATYPE(SongData)` after the complete type, register it
before connecting the worker, connect `QMediaPlayer::playbackStateChanged` to
the state updater, and reset player/source/selection/progress/duration/position
before model reset.

Validate APIC frame length before every indexed access, cap embedded artwork at
8 MiB, honor the declared MIME type, and ignore invalid artwork. Implement
`scrubTo(qreal ratio)` as the only UI scrub command.

- [ ] **Step 4: Make tests guiless and verify GREEN**

Replace `QTEST_MAIN` with `QTEST_GUILESS_MAIN` where multimedia behavior under
test does not need a GUI. Avoid starting actual playback in unit tests.

Run: `cmake --build build --target tst_music_playback -j2 && ./build/tests/tst_music_playback`

Expected: all music tests PASS without requiring PipeWire, PulseAudio, or a display.

- [ ] **Step 5: Commit**

```bash
git add src/services/MusicScanner.* src/viewmodels/MusicPlayerViewModel.* tests
git commit -m "fix: harden music scanning and playback state"
```

### Task 4: Restore Zero-JavaScript QML and layout correctness

**Files:**
- Modify: `qml/components/MusicPlayer.qml`
- Modify: `qml/components/EnergyBlocks.qml`
- Modify: `qml/components/NeonIconButton.qml`
- Modify: `qml/components/NeonTickGauge.qml`
- Modify: `qml/screens/DashboardScreen.qml`

**Interfaces:**
- Consumes: `MusicPlayerViewModel::scrubTo(qreal)`.
- Produces: passive QML with stable sizing and non-recursive effects.

- [ ] **Step 1: Capture failing static checks**

Run:

```bash
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py $(rg --files -g '*.qml')
rg -n 'on[A-Z][A-Za-z]+:\\s*\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml
```

Expected: scrubber mutation blocks and current QML lint findings are reported.

- [ ] **Step 2: Make scrubber handlers passive**

Replace multi-statement blocks with single direct invocations such as:

```qml
onPressed: MusicViewModel.scrubTo(mouse.x / width)
onPositionChanged: MusicViewModel.scrubTo(mouse.x / width)
```

No QML state assignment is permitted.

- [ ] **Step 3: Correct geometry and effect ownership**

Give the track-info column an explicit width from its containing item, propagate
the `Row` implicit size through `EnergyBlocks`, use implicit dimensions on
`NeonTickGauge`, point the backdrop `MultiEffect` at the isolated image source,
mark internal derived colors readonly, and base image fallback on declarative
`Image.status`.

- [ ] **Step 4: Reduce confirmed hot-path cost**

Remove per-tick color/opacity `Behavior` objects where the enclosing displayed
value already animates, use `Text.PlainText` for numeric tick labels, remove
unused IDs, and avoid keeping the error toast renderable after its state clears.

- [ ] **Step 5: Verify QML**

Run the static checks from Step 1 and:

`cmake --build build --target QtStmAutomotiveSimulator -j2`

Expected: no Zero-JS violations, QML cache compilation succeeds, and no critical changed-scope QML review findings remain.

- [ ] **Step 6: Commit**

```bash
git add qml
git commit -m "fix: restore passive and stable QML views"
```

### Task 5: Document the mock-only repository

**Files:**
- Create: `docs/repository_tree.md`
- Modify: `docs/architecture.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/tasks_board.md`
- Modify: `AGENTS.md` if serial fallback is still stated as current behavior

**Interfaces:**
- Produces: an authoritative tree and current-runtime documentation.

- [ ] **Step 1: Generate the maintained tree**

Use `rg --files` while excluding `.git` and `build`. Document every source,
QML component, test, resource folder, workflow folder, and primary document
with a one-line responsibility.

- [ ] **Step 2: Update architecture truth**

Describe simulator-only startup and mark hardware integration as inactive
historical/future reference. Remove claims that runtime fallback, watchdog, or
SerialService currently exists. Keep the project name unchanged.

- [ ] **Step 3: Update the task board**

Add a verified mock-only consolidation phase. Do not mark build/test/review
items complete until Task 6 evidence exists.

- [ ] **Step 4: Validate documentation**

Run:

```bash
rg -n 'SerialService|Qt6::SerialPort|runtime fallback|auto-reconnect' CMakeLists.txt src qml tests docs AGENTS.md
```

Expected: matches occur only in clearly labelled historical/future documentation or the design/plan history.

- [ ] **Step 5: Commit**

```bash
git add AGENTS.md docs
git commit -m "docs: describe the mock-only repository"
```

### Task 6: Final verification and delivery

**Files:**
- Modify only files required by verification failures in the changed scope.

**Interfaces:**
- Produces: fresh build, test, lint, review, and Git evidence.

- [ ] **Step 1: Clean configure and build**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
```

Expected: exit code `0`; no compiler warnings originating from changed files.

- [ ] **Step 2: Run all tests**

Run: `ctest --test-dir build --output-on-failure`

Expected: all tests PASS without display/audio/hardware dependencies.

- [ ] **Step 3: Run deterministic reviews**

Run both project review linters over every C++ and QML source. Resolve
changed-scope critical/high-confidence findings; record unrelated remaining
style findings explicitly.

- [ ] **Step 4: Verify requirements and repository state**

Run:

```bash
git diff --check
git status --short
rg -n 'SerialService|Qt6::SerialPort' CMakeLists.txt src qml tests
rg -n 'on[A-Z][A-Za-z]+:\\s*\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml
```

Expected: no whitespace errors, no active serial code, no imperative QML logic,
and only intended changes.

- [ ] **Step 5: Commit final verification corrections**

```bash
git add <verified-files>
git commit -m "chore: complete mock-only verification"
```

- [ ] **Step 6: Push**

Run: `git push`

Expected: all implementation commits are present on the configured upstream.

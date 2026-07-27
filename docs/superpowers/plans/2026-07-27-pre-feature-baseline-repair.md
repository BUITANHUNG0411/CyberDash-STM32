# Pre-Feature Baseline Repair Implementation Plan

> **Status:** Historical repair plan retained for reference after the baseline was restored.
> The current active focus is the encoder-driven arrow-road scene.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore a verified baseline in which runtime behavior, automated tests, Zero-JavaScript policy, and project documentation agree.

**Architecture:** Extract newline/checksum parsing into a hardware-free parser, emit raw serial telemetry, and map it to the existing dashboard contract in C++. Move scrubber interaction state and coordinate normalization into `MusicPlayerViewModel`, while allowing multimedia construction to be disabled in unit tests. Update documentation only after the code behavior is verified.

**Tech Stack:** C++17, Qt 6.8+ Core/Gui/QML/Quick/Test/SerialPort/Multimedia, QML, CMake 3.16+, Qt Test.

## Global Constraints

- QML contains no imperative JavaScript blocks, local state mutation, or JavaScript utility calls.
- QML remains unchanged when switching between `SimulatorService` and `SerialService`.
- Reconnect open success does not claim a valid hardware connection until a checksummed telemetry frame arrives.
- No STM32 firmware or UART wire-format change is introduced.
- Every behavior change follows a witnessed RED → GREEN test cycle.
- Documentation completion claims are written only after fresh verification.

---

## File Map

- Create `src/services/SerialTelemetryParser.h`: raw telemetry value type and incremental newline/checksum parser.
- Create `src/services/SerialTelemetryParser.cpp`: parser implementation with 4096-byte buffer limit.
- Create `src/services/TelemetryMapper.h`: dashboard telemetry value type and pure STM32-to-dashboard mapper.
- Create `src/services/TelemetryMapper.cpp`: speed, gear, warning, and placeholder-field mapping previously embedded in `SerialService`.
- Create `tests/tst_serial_pipeline.cpp`: parser, mapper, resource-error, and watchdog regression tests.
- Modify `src/services/SerialService.{h,cpp}`: transport-only signal contract and idempotent connection transitions.
- Modify `src/main.cpp`: map raw serial frames before updating the existing ViewModels.
- Modify `src/viewmodels/MusicPlayerViewModel.{h,cpp}`: C++ scrubber state and test-safe multimedia construction.
- Modify `qml/components/MusicPlayer.qml`: passive bindings and one-call event handlers.
- Modify `tests/tst_music_playback.cpp`: scrubber and headless construction tests.
- Modify `tests/CMakeLists.txt`: serial test target and deterministic headless test environment.
- Modify `CMakeLists.txt`: compile new service sources.
- Modify `AGENTS.md`, `README.md`, `docs/*.md`: synchronize verified project truth.

---

### Task 1: Extract and Test the Serial Parser and Telemetry Mapper

**Files:**

- Create: `src/services/SerialTelemetryParser.h`
- Create: `src/services/SerialTelemetryParser.cpp`
- Create: `src/services/TelemetryMapper.h`
- Create: `src/services/TelemetryMapper.cpp`
- Create: `tests/tst_serial_pipeline.cpp`
- Modify: `tests/CMakeLists.txt`
- Modify: `CMakeLists.txt`

**Interfaces:**

- Produces: `struct RawSerialTelemetry { int rpm; double batteryVoltage; int errorCode; };`
- Produces: `QList<RawSerialTelemetry> SerialTelemetryParser::append(const QByteArray &bytes)`
- Produces: `void SerialTelemetryParser::clear()`
- Produces: `struct DashboardTelemetry { double speed; int rpm; QString gear; bool warning; int battery; int range; int temperature; };`
- Produces: `DashboardTelemetry TelemetryMapper::fromSerial(const RawSerialTelemetry &raw)`

- [ ] **Step 1: Write failing parser and mapper tests**

Add focused Qt Test cases to `tests/tst_serial_pipeline.cpp`:

```cpp
void validFrameIsAccepted()
{
    SerialTelemetryParser parser;
    const auto frames = parser.append("TEL,118,11.8,0;129\n");
    QCOMPARE(frames.size(), 1);
    QCOMPARE(frames.first().rpm, 118);
    QCOMPARE(frames.first().batteryVoltage, 11.8);
    QCOMPARE(frames.first().errorCode, 0);
}

void invalidChecksumIsRejected()
{
    SerialTelemetryParser parser;
    QCOMPARE(parser.append("TEL,118,11.8,0;128\n").size(), 0);
}

void partialFrameIsAccumulated()
{
    SerialTelemetryParser parser;
    QCOMPARE(parser.append("TEL,118,11.").size(), 0);
    QCOMPARE(parser.append("8,0;129\n").size(), 1);
}

void oversizedInputIsDiscarded()
{
    SerialTelemetryParser parser;
    QCOMPARE(parser.append(QByteArray(4097, 'X')).size(), 0);
    QCOMPARE(parser.append("TEL,118,11.8,0;129\n").size(), 1);
}

void rawTelemetryMapsOutsideTransport()
{
    const auto value = TelemetryMapper::fromSerial({3000, 11.8, 0});
    QCOMPARE(value.speed, 90.0);
    QCOMPARE(value.rpm, 3000);
    QCOMPARE(value.gear, QString("5"));
    QCOMPARE(value.warning, false);
    QCOMPARE(value.battery, 100);
    QCOMPARE(value.range, 325);
    QCOMPARE(value.temperature, 57);
}
```

- [ ] **Step 2: Register and run the test to verify RED**

Add a `tst_serial_pipeline` target linked to `Qt6::Core`, `Qt6::Test`, and `Qt6::SerialPort`, then run:

```bash
cmake -S . -B build
cmake --build build --target tst_serial_pipeline -j2
ctest --test-dir build -R tst_serial_pipeline --output-on-failure
```

Expected: build failure because the parser and mapper headers do not exist.

- [ ] **Step 3: Implement the minimal parser**

Implement incremental buffering and parse only complete newline-terminated frames:

```cpp
QList<RawSerialTelemetry> SerialTelemetryParser::append(const QByteArray &bytes)
{
    m_buffer.append(bytes);
    if (m_buffer.size() > 4096) {
        m_buffer.clear();
        return {};
    }

    QList<RawSerialTelemetry> result;
    while (m_buffer.contains('\n')) {
        const qsizetype newline = m_buffer.indexOf('\n');
        const QString line = QString::fromUtf8(m_buffer.first(newline)).trimmed();
        m_buffer.remove(0, newline + 1);
        const auto parsed = parseLine(line);
        if (parsed.has_value())
            result.append(*parsed);
    }
    return result;
}
```

`parseLine()` accepts exactly `TEL,<rpm>,<vbat>,<error>;<checksum>` and validates `(rpm + static_cast<int>(vbat) + error) & 0xFF`.

- [ ] **Step 4: Implement the pure telemetry mapper**

Move the current speed/gear/warning behavior verbatim into `TelemetryMapper::fromSerial()`:

```cpp
DashboardTelemetry TelemetryMapper::fromSerial(const RawSerialTelemetry &raw)
{
    const double speed = static_cast<double>(raw.rpm) * 0.03;
    QString gear = QStringLiteral("N");
    if (speed > 80.0) gear = QStringLiteral("5");
    else if (speed > 60.0) gear = QStringLiteral("4");
    else if (speed > 40.0) gear = QStringLiteral("3");
    else if (speed > 20.0) gear = QStringLiteral("2");
    else if (speed > 0.0) gear = QStringLiteral("1");

    return {speed, raw.rpm, gear,
            raw.errorCode != 0 || raw.batteryVoltage < 10.5,
            100, 325, 57};
}
```

- [ ] **Step 5: Verify GREEN**

Run:

```bash
cmake --build build --target tst_serial_pipeline -j2
ctest --test-dir build -R tst_serial_pipeline --output-on-failure
```

Expected: `tst_serial_pipeline` passes all parser and mapper cases.

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt tests/CMakeLists.txt tests/tst_serial_pipeline.cpp \
  src/services/SerialTelemetryParser.h src/services/SerialTelemetryParser.cpp \
  src/services/TelemetryMapper.h src/services/TelemetryMapper.cpp
git commit -m "test: cover serial parsing and telemetry mapping"
```

---

### Task 2: Make Serial Fallback Deterministic

**Files:**

- Modify: `src/services/SerialService.h`
- Modify: `src/services/SerialService.cpp`
- Modify: `src/main.cpp`
- Modify: `tests/tst_serial_pipeline.cpp`

**Interfaces:**

- Consumes: `SerialTelemetryParser`
- Consumes: `TelemetryMapper::fromSerial(const RawSerialTelemetry &)`
- Produces signal: `void rawTelemetryUpdated(int rpm, double batteryVoltage, int errorCode)`
- Produces private transition: `void setConnected(bool connected)`
- Produces state: `bool m_connectionStateKnown = false`

- [ ] **Step 1: Add failing connection-state tests**

Extend `tst_serial_pipeline.cpp`:

```cpp
void resourceErrorEmitsDisconnectedOnce()
{
    SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
    QSignalSpy spy(&service, &SerialService::connectionStatusChanged);
    QVERIFY(QMetaObject::invokeMethod(
        &service, "handleError", Qt::DirectConnection,
        Q_ARG(QSerialPort::SerialPortError, QSerialPort::ResourceError)));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), false);
}

void validBytesEmitRawTelemetry()
{
    SerialService service(QStringLiteral("/definitely/not/a/serial/port"));
    QSignalSpy spy(&service, &SerialService::rawTelemetryUpdated);
    QVERIFY(QMetaObject::invokeMethod(
        &service, "processIncomingBytes", Qt::DirectConnection,
        Q_ARG(QByteArray, QByteArray("TEL,118,11.8,0;129\n"))));
    QCOMPARE(spy.count(), 1);
}
```

Register `QSerialPort::SerialPortError` if required for queued/meta-object invocation.

- [ ] **Step 2: Run the tests to verify RED**

```bash
cmake --build build --target tst_serial_pipeline -j2
ctest --test-dir build -R tst_serial_pipeline --output-on-failure
```

Expected: compile failure because `rawTelemetryUpdated` and `processIncomingBytes` do not exist.

- [ ] **Step 3: Replace dashboard telemetry emission with raw telemetry**

`handleReadyRead()` delegates to a shared slot:

```cpp
void SerialService::handleReadyRead()
{
    processIncomingBytes(m_serial->readAll());
}

void SerialService::processIncomingBytes(const QByteArray &bytes)
{
    const auto frames = m_parser.append(bytes);
    for (const RawSerialTelemetry &frame : frames) {
        m_watchdogTimer->start();
        setConnected(true);
        emit rawTelemetryUpdated(frame.rpm, frame.batteryVoltage, frame.errorCode);
    }
}
```

Remove speed, gear, warning, battery, range, and temperature derivation from `SerialService`.

- [ ] **Step 4: Converge all disconnect paths**

Implement an idempotent transition:

```cpp
void SerialService::setConnected(bool connected)
{
    if (m_connectionStateKnown && m_isConnected == connected)
        return;
    m_connectionStateKnown = true;
    m_isConnected = connected;
    emit connectionStatusChanged(connected);
}
```

The `m_connectionStateKnown` flag allows startup failure to emit one initial
`false` notification while suppressing duplicate disconnected notifications.
Resource error and watchdog call `setConnected(false)`, close the port safely,
and start the reconnect timer. Reconnect open success starts the watchdog but
does not call `setConnected(true)`.

- [ ] **Step 5: Map raw serial telemetry in `main.cpp`**

```cpp
QObject::connect(
    &serialService, &SerialService::rawTelemetryUpdated,
    [&](int rpm, double batteryVoltage, int errorCode) {
        if (!isHardwareConnected)
            return;
        const DashboardTelemetry data =
            TelemetryMapper::fromSerial({rpm, batteryVoltage, errorCode});
        vm.updateTelemetry(data.speed, data.rpm, data.gear, data.warning,
                           data.battery, data.range, data.temperature);
        tripVm.updateSpeed(data.speed, tripClock.restart());
    });
```

The simulator connection remains unchanged.

- [ ] **Step 6: Verify GREEN and regression build**

```bash
cmake --build build -j2
ctest --test-dir build -R 'tst_serial_pipeline|tst_viewmodels' --output-on-failure
```

Expected: build succeeds and both selected tests pass.

- [ ] **Step 7: Commit**

```bash
git add src/services/SerialService.h src/services/SerialService.cpp \
  src/main.cpp tests/tst_serial_pipeline.cpp
git commit -m "fix: make serial fallback deterministic"
```

---

### Task 3: Move Music Scrubber State to C++ and Stabilize Headless Tests

**Files:**

- Modify: `src/viewmodels/MusicPlayerViewModel.h`
- Modify: `src/viewmodels/MusicPlayerViewModel.cpp`
- Modify: `qml/components/MusicPlayer.qml`
- Modify: `tests/tst_music_playback.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**

- Produces property: `bool scrubberDragging READ scrubberDragging NOTIFY scrubberStateChanged`
- Produces property: `float scrubberRatio READ scrubberRatio NOTIFY scrubberStateChanged`
- Produces invokables: `beginScrub(qreal position, qreal width)`, `updateScrub(qreal position, qreal width)`, `endScrub()`
- Produces constructor: `MusicPlayerViewModel(QObject *parent = nullptr, bool multimediaEnabled = true)`

- [ ] **Step 1: Write failing scrubber and headless tests**

Construct the ViewModel with multimedia disabled:

```cpp
void scrubber_clamps_and_tracks_test()
{
    MusicPlayerViewModel vm(nullptr, false);
    QSignalSpy spy(&vm, &MusicPlayerViewModel::scrubberStateChanged);

    vm.beginScrub(150.0, 100.0);
    QCOMPARE(vm.scrubberDragging(), true);
    QCOMPARE(vm.scrubberRatio(), 1.0f);

    vm.updateScrub(-10.0, 100.0);
    QCOMPARE(vm.scrubberRatio(), 0.0f);

    vm.endScrub();
    QCOMPARE(vm.scrubberDragging(), false);
    QVERIFY(spy.count() >= 3);
}

void scrubber_ignores_zero_width_test()
{
    MusicPlayerViewModel vm(nullptr, false);
    vm.beginScrub(20.0, 0.0);
    QCOMPARE(vm.scrubberDragging(), true);
    QCOMPARE(vm.scrubberRatio(), 0.0f);
}
```

Change all existing tests to `MusicPlayerViewModel vm(nullptr, false);`.

- [ ] **Step 2: Run to verify RED**

```bash
cmake --build build --target tst_music_playback -j2
QT_QPA_PLATFORM=offscreen ctest --test-dir build -R tst_music_playback --output-on-failure
```

Expected: compile failure because the constructor and scrubber API do not exist.

- [ ] **Step 3: Implement test-safe multimedia lifecycle**

When `multimediaEnabled` is false, do not construct `QMediaPlayer` or `QAudioOutput`. Guard media-dependent operations and preserve pure state operations:

```cpp
if (multimediaEnabled) {
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    // Existing media signal connections.
}
```

Initialize media pointers to `nullptr`; getters return safe stopped/default state when disabled. Scanner behavior remains available and its thread still shuts down in the destructor.

- [ ] **Step 4: Implement C++ scrubber behavior**

```cpp
void MusicPlayerViewModel::beginScrub(qreal position, qreal width)
{
    m_scrubberDragging = true;
    updateScrub(position, width);
}

void MusicPlayerViewModel::updateScrub(qreal position, qreal width)
{
    if (width <= 0.0)
        return;
    const float ratio = std::clamp(
        static_cast<float>(position / width), 0.0f, 1.0f);
    if (!qFuzzyCompare(m_scrubberRatio, ratio)) {
        m_scrubberRatio = ratio;
        emit scrubberStateChanged();
    }
    seek(ratio);
}

void MusicPlayerViewModel::endScrub()
{
    if (!m_scrubberDragging)
        return;
    m_scrubberDragging = false;
    emit scrubberStateChanged();
}
```

Ensure `beginScrub()` emits when drag state changes even if the ratio remains zero.

- [ ] **Step 5: Replace imperative QML blocks**

Remove local `scrubberDragging`, `dragX`, and `Math.max`. Bind the progress width to C++:

```qml
width: parent.width * (MusicViewModel.scrubberDragging
                       ? MusicViewModel.scrubberRatio
                       : MusicViewModel.progress)
```

Use direct one-call handlers:

```qml
onPressed: MusicViewModel.beginScrub(mouseX, width)
onPositionChanged: MusicViewModel.updateScrub(mouseX, width)
onReleased: MusicViewModel.endScrub()
onCanceled: MusicViewModel.endScrub()
```

- [ ] **Step 6: Configure deterministic CTest environment**

Set test properties in `tests/CMakeLists.txt`:

```cmake
set_tests_properties(tst_music_playback PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 20
)
```

Because multimedia is disabled by the tests, no PipeWire/PulseAudio connection is attempted.

- [ ] **Step 7: Verify GREEN and Zero-JavaScript**

```bash
cmake --build build --target tst_music_playback -j2
ctest --test-dir build -R tst_music_playback --output-on-failure
rg -n '\\b(function|if|for|while|switch|var|let|const)\\b|on[A-Z][A-Za-z]+\\s*:\\s*\\{' qml -g '*.qml'
```

Expected: music tests pass; the QML scan returns no code matches. Comments containing words such as “for” must be excluded or reviewed manually rather than treated as failures.

- [ ] **Step 8: Commit**

```bash
git add src/viewmodels/MusicPlayerViewModel.h \
  src/viewmodels/MusicPlayerViewModel.cpp \
  qml/components/MusicPlayer.qml tests/tst_music_playback.cpp \
  tests/CMakeLists.txt
git commit -m "fix: move music scrubber interaction to C++"
```

---

### Task 4: Synchronize Core Project Documentation

**Files:**

- Modify: `AGENTS.md`
- Modify: `README.md`
- Modify: `docs/DOCUMENTATION_STANDARDS.md`
- Modify: `docs/architecture.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`

**Interfaces:**

- Consumes: verified parser, mapper, connection, scrubber, build, and test behavior from Tasks 1–3.
- Produces: one consistent source of truth for future agents and contributors.

- [ ] **Step 1: Update `AGENTS.md`**

Apply these exact truth corrections:

- Make the target agent/tool-neutral.
- State C++17, matching CMake.
- List only Bike, Scooter, and Car vehicle modes.
- Mark STM32/UART support as implemented but pending field validation.
- Point design inspiration to `resources/media/dashboard-preview.png`.
- Replace configure-only Golden Check with configure, build, CTest, Zero-JS scan, and QML review.
- Make commit/push conditional on successful verification and repository/user authorization.

- [ ] **Step 2: Expand architecture and hardware guides**

Document:

- Seven context ViewModels and their ownership in `main.cpp`.
- Simulator/full-dashboard vs serial/raw-telemetry signal contracts.
- `SerialTelemetryParser` and `TelemetryMapper` boundaries.
- Exact checksum `(rpm + int(vbat) + error) & 0xFF`.
- Valid example `TEL,118,11.8,0;129\n`.
- 4096-byte buffer, 500 ms watchdog, 2000 ms reconnect.
- Resource error → disconnected → simulator state flow.
- GUI-thread `QSerialPort::readyRead` parsing and worker-thread-only music scanning.
- Troubleshooting sections required by the documentation standard.

- [ ] **Step 3: Update testing and UI guides**

Testing documentation must list:

- `tst_viewmodels`, `tst_music_playback`, and `tst_serial_pipeline`.
- Headless environment configured by CTest.
- Parser, mapper, connection-state, Q_PROPERTY, and scrubber coverage.
- Zero-JS scan, QML review, build, CTest, and smoke commands.

UI documentation must list:

- Day/night palette and ECO/NORMAL/SPORT accents.
- Car/Bike/Scooter states.
- CenterHub Music/Map.
- Dip transition and sibling-source `MultiEffect`.
- Current screenshot as the available canonical reference.
- A valid numeric animation example instead of animating `Text.text`.

- [ ] **Step 4: Correct README and historical status**

README must:

- Remove nonexistent `CLAUDE.md` and `docs/adr/`.
- Show Phase 0–17 status and all current ViewModels/components/tests.
- Describe actual test responsibilities.
- Use valid checksummed UART examples.
- Include deterministic test commands.

`tasks_board.md` must preserve phase history but correct false claims about deleting `GlassPanel`/`setScenario`, moving telemetry, and Zero-JS. Add Phase 17 “Pre-Feature Baseline Repair” and mark items only from fresh evidence.

`journal.md` records why the baseline repair was required and the final boundary decisions.

- [ ] **Step 5: Clarify Markdown standards**

State that active guides require H1 plus `AI Context`, while dated historical specs/plans may use their workflow-mandated metadata block. Require language tags on opening code fences and Troubleshooting in active technical guides.

- [ ] **Step 6: Validate documentation mechanically**

```bash
test -f resources/media/dashboard-preview.png
! rg -n 'docs/assets/inspiration-design\\.webp|Phase 0–11|CLAUDE\\.md|docs/adr/' \
  AGENTS.md README.md docs/*.md
rg -n '^# ' AGENTS.md README.md docs/*.md
git diff --check
```

Expected: asset exists, stale-reference scan returns no matches, active documents have H1 headings, and diff check is clean.

- [ ] **Step 7: Commit**

```bash
git add AGENTS.md README.md docs/DOCUMENTATION_STANDARDS.md \
  docs/architecture.md docs/hardware_integration.md docs/testing_strategy.md \
  docs/ui_ux_guidelines.md docs/tasks_board.md docs/journal.md
git commit -m "docs: synchronize project baseline with verified code"
```

---

### Task 5: Full Verification, Review, and Final Status

**Files:**

- Modify if evidence requires: `docs/tasks_board.md`
- Modify if evidence requires: `docs/journal.md`

**Interfaces:**

- Consumes: all implementation and documentation from Tasks 1–4.
- Produces: fresh verification evidence and final Phase 17 status.

- [ ] **Step 1: Configure and build from the supported build tree**

```bash
cmake -S . -B build
cmake --build build -j2
```

Expected: exit code 0 with no compiler warnings.

- [ ] **Step 2: Run the complete test suite**

```bash
ctest --test-dir build --output-on-failure
```

Expected: `tst_viewmodels`, `tst_music_playback`, and `tst_serial_pipeline` all pass.

- [ ] **Step 3: Run Zero-JavaScript and QML review checks**

Run the repository `qt-qml-review` skill and the focused scan:

```bash
rg -n 'on[A-Z][A-Za-z]+\\s*:\\s*\\{|\\bMath\\.|\\b(function|var|let|const)\\b' \
  qml -g '*.qml'
```

Expected: no imperative QML handler blocks, JavaScript utility calls, functions, or mutable JavaScript declarations.

- [ ] **Step 4: Run C++ review**

Run the repository `qt-cpp-review` skill over the modified C++ files. Resolve only high-confidence findings in scope, using TDD for behavior changes, then repeat build and tests.

- [ ] **Step 5: Run bounded offscreen smoke launch**

```bash
timeout 8s env QT_QPA_PLATFORM=offscreen \
  ./build/QtStmAutomotiveSimulator
```

Expected: process reaches the event loop, times out with code 124, and prints no QML load/type/binding errors. Audio-service warnings are documented separately if the production media backend probes unavailable host services.

- [ ] **Step 6: Record only verified final status**

If every required command passes, mark Phase 17 verification items `[x]` and record exact test target counts in `docs/journal.md`. If any command fails, leave the corresponding task unchecked and document the observed blocker without claiming completion.

- [ ] **Step 7: Commit final evidence**

```bash
git add docs/tasks_board.md docs/journal.md
git commit -m "docs: record baseline repair verification"
```

- [ ] **Step 8: Push the verified branch**

```bash
git push origin feature/center-hub-map
```

Expected: the remote branch advances through the final verification commit. If remote access requires approval, request it rather than bypassing the sandbox.

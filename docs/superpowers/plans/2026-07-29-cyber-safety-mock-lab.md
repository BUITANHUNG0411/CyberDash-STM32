# Cyber Safety Mock Lab Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `subagent-driven-development` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a deterministic, mock-only Cyber Safety Event Simulator that delivers a 72-second normal → advisory → critical → recovery dashboard cinematic without changing UART, telemetry, Music, Parking Assist, or vehicle-mode contracts.

**Architecture:** A GUI-thread `MockSafetyScenarioService` owns an immutable elapsed-time script and a 100 ms production timer. `SafetyScenarioViewModel` converts that script into presentation-safe `Q_PROPERTY` values and owns the direct QML invokables. `SafetyScenarioOverlay.qml` is a passive sibling of `CenterHub` in the existing Car center panel; `main.cpp` only owns/wires the new pair and gates it when Car mode is unavailable or Parking is live-critical.

**Tech Stack:** Qt 6.8+, C++17, Qt Core/Qt Test, Qt Quick/QML, CMake, no additional dependencies.

## Global Constraints

- QML is a passive declarative view: no imperative JavaScript, local interaction state, calculations, timers, functions, or block handlers.
- Keep all scenario state, timeline boundaries, copy, risk segment count, acknowledgement policy, and availability gating in C++ through `Q_PROPERTY`/`Q_INVOKABLE`.
- The feature is mock-only: display `DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL` while active; do not claim sensing, ADAS, automatic braking, or functional safety.
- Do not edit `SerialService`, `SerialTelemetryParser`, `TelemetryMapper`, the UART protocol, or the Simulator↔Serial source gate.
- Do not add a CenterHub page or a `DashboardScreen.state`; preserve the Music/Parking two-page contract and Car/Bike/Scooter state machine.
- Reuse `Theme.accentCyan`, `Theme.parkingCaution`, `Theme.warningRed`, glass components, and numeric/opacity/scale animations. Do not add Qt Quick 3D, Location, camera/video, SerialBus, networking, or a recursive `MultiEffect`.
- Every property emits its notify signal only when its effective value changes. Production timer work remains bounded on the GUI thread; tests drive elapsed time directly.
- Preserve all pre-existing dirty-worktree changes. Do not stage, commit, reset, checkout, or delete files; the user has not authorized version-control mutations.

## Decision Record

- **Selected:** a separate Safety Scenario service and ViewModel, rather than extending `MockScenarioEngine`. Its current ErrorInjection randomness and non-resetting scenario clock make it unsuitable as the deterministic MVP boundary.
- **Selected:** a single abstract forward-hazard script, not camera, maps, lanes, or multiple scenarios. It gives the strongest first-ten-second visual with the smallest deterministic model.
- **Selected:** an overlay sibling of CenterHub, not a third page or vehicle state. Music remains alive; Parking stays authoritative for its existing critical condition.
- **Selected:** C++ availability gating. Leaving Car mode or receiving a live critical parking sample stops the lab before it can obscure the established warning surface.

---

### Task 1: Deterministic safety-script service (TDD)

**Files:**
- Create: `src/services/MockSafetyScenarioService.h`
- Create: `src/services/MockSafetyScenarioService.cpp`
- Create: `tests/tst_safety_scenario.cpp`
- Modify: `tests/CMakeLists.txt`

**Consumes:** Qt Core and Qt Test only.

**Produces:** `MockSafetyScenarioService`, with a deterministic timeline driven by `start()`, `stop()`, `replay()`, `acknowledge()`, and `advance(qint64 elapsedMs)`.

**Required public interface:**

```cpp
class MockSafetyScenarioService final : public QObject
{
    Q_OBJECT
public:
    enum class Phase { Idle, Normal, Advisory, Critical, Recovery, Complete };
    Q_ENUM(Phase)
    enum class Severity { None, Advisory, Critical };
    Q_ENUM(Severity)

    explicit MockSafetyScenarioService(QObject *parent = nullptr);
    static constexpr qint64 updateIntervalMs() { return 100; }
    void start();
    void stop();
    void replay();
    bool acknowledge();
    void advance(qint64 elapsedMs);
    bool isRunning() const;
    Phase phase() const;
    Severity severity() const;
    qreal riskProgress() const;
    int riskSegments() const;
    qreal threatPosition() const;
    bool pulseActive() const;
    bool acknowledgementAvailable() const;

signals:
    void frameChanged();
    void runningChanged();
};
```

- [ ] **Step 1: Write the failing service tests and register the target.**

  Add `tst_safety_scenario` to `tests/CMakeLists.txt`, initially compiling the test and the future service source. Write tests that express these exact timeline boundaries:

  ```cpp
  void startsInNormalFrame();
  void phaseBoundaries_data();
  void phaseBoundaries();
  void acknowledgementOnlyWorksInAcknowledgementWindow();
  void replayResetsTheElapsedTimeline();
  void nonPositiveAdvanceDoesNotChangeTheFrame();
  void frameNotificationsAreSuppressedForUnchangedState();
  ```

  The data rows must assert: `0`/`5999` = Normal + None; `6000`/`19999` = Advisory + Advisory; `20000`/`47999` = Critical + Critical; `48000`/`71999` = Recovery + Advisory; `72000` = Complete + None. Assert `acknowledgementAvailable()` only from `34000` through `47999`. Assert a replay after `advance(20000)` returns Normal, risk `0.0`, and zero lit segments.

- [ ] **Step 2: Verify RED.**

  Run:

  ```bash
  cmake -S . -B build
  cmake --build build --target tst_safety_scenario -j2
  ```

  Expected: compilation fails because `services/MockSafetyScenarioService.h` and its implementation do not exist yet. The failure proves the test is targeting absent behavior, not an existing scenario engine.

- [ ] **Step 3: Implement the smallest deterministic service.**

  Use constants `6000`, `20000`, `34000`, `48000`, and `72000` ms. `start()` and `replay()` reset elapsed time to zero, mark running, compute Normal, and notify only effective changes. `advance()` ignores non-positive values, caps elapsed time at 72000 ms, and updates one complete frame. `acknowledge()` returns `true` only in the 34000–47999 ms critical window, moves elapsed time to 48000 ms, and publishes Recovery.

  Derive all visual data in C++: risk is `0.0` in Normal/Complete, grows from `0.20` to `0.70` during Advisory, is `1.0` during Critical, and decays from `1.0` to `0.0` in Recovery; segments are bounded `0..8`; threat position is bounded `0.0..1.0`; pulse is true only in Critical. A parented `QTimer` at 100 ms must call `advance(updateIntervalMs())`; tests never need its event loop.

- [ ] **Step 4: Verify GREEN.**

  Run:

  ```bash
  cmake --build build --target tst_safety_scenario -j2
  ctest --test-dir build -R tst_safety_scenario --output-on-failure
  ```

  Expected: the new target passes all service timeline, reset, boundary, and signal-count tests.

### Task 2: Safety Scenario ViewModel (TDD)

**Files:**
- Create: `src/viewmodels/SafetyScenarioViewModel.h`
- Create: `src/viewmodels/SafetyScenarioViewModel.cpp`
- Modify: `tests/tst_safety_scenario.cpp`
- Modify: `tests/CMakeLists.txt`

**Consumes:** `MockSafetyScenarioService` from Task 1.

**Produces:** a dedicated C++ presentation and interaction boundary, without changing `VehicleStatusViewModel` or `CenterHubViewModel`.

**Required QML contract:**

```cpp
Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
Q_PROPERTY(bool presentationVisible READ presentationVisible NOTIFY presentationVisibleChanged)
Q_PROPERTY(bool canStart READ canStart NOTIFY canStartChanged)
Q_PROPERTY(QString title READ title NOTIFY displayChanged)
Q_PROPERTY(QString instructionText READ instructionText NOTIFY displayChanged)
Q_PROPERTY(QString disclaimerText READ disclaimerText CONSTANT)
Q_PROPERTY(qreal riskProgress READ riskProgress NOTIFY riskProgressChanged)
Q_PROPERTY(int riskSegments READ riskSegments NOTIFY riskSegmentsChanged)
Q_PROPERTY(qreal threatPosition READ threatPosition NOTIFY threatPositionChanged)
Q_PROPERTY(bool advisoryActive READ advisoryActive NOTIFY advisoryActiveChanged)
Q_PROPERTY(bool criticalActive READ criticalActive NOTIFY criticalActiveChanged)
Q_PROPERTY(bool pulseActive READ pulseActive NOTIFY pulseActiveChanged)
Q_PROPERTY(bool acknowledgementAvailable READ acknowledgementAvailable NOTIFY acknowledgementAvailableChanged)
```

`startDemo()`, `replay()`, `acknowledge()`, and `stopDemo()` are `Q_INVOKABLE`. `setPresentationAllowed(bool)` is C++-only and stops an active script when it becomes false.

- [ ] **Step 1: Extend the test target with failing ViewModel tests.**

  Add tests that construct the real service and VM together:

  ```cpp
  void viewModelMapsScriptToPresentationText();
  void acknowledgementTransitionsToRecoveryThroughTheViewModel();
  void availabilityGateStopsAndHidesTheLab();
  void effectivePropertyNotificationsAreNotDuplicated();
  ```

  Assert exact copy: Normal title `SAFETY LAB` with instruction `FORWARD HAZARD SIMULATION`; Advisory title `FORWARD HAZARD` with instruction `CAUTION: CLOSING DISTANCE`; Critical title `BRAKE ADVISORY`; Recovery title `RISK CLEARING`; disclaimer `DEMO ONLY — NO REAL SENSOR / NO VEHICLE CONTROL`. Assert `presentationVisible` and `canStart` become false when availability is disabled and that `active` becomes false.

- [ ] **Step 2: Verify RED.**

  Run:

  ```bash
  cmake --build build --target tst_safety_scenario -j2
  ```

  Expected: compilation fails because `SafetyScenarioViewModel` does not yet exist.

- [ ] **Step 3: Implement the ViewModel.**

  Connect service `frameChanged` and `runningChanged` to a private synchronization routine. Snapshot every prior effective property, read the service frame, derive the four prescribed titles/instructions and boolean presentation roles, then emit each notify signal only when its corresponding effective value changed. `presentationVisible` is exactly `active && presentationAllowed`; `canStart` is exactly `presentationAllowed && !active`. The ViewModel must not write dashboard telemetry, select CenterHub pages, or access serial objects.

- [ ] **Step 4: Verify GREEN.**

  Run:

  ```bash
  cmake --build build --target tst_safety_scenario -j2
  ctest --test-dir build -R tst_safety_scenario --output-on-failure
  ```

  Expected: service and ViewModel tests pass, including QSignalSpy notification suppression.

### Task 3: Application wiring and passive Car overlay

**Files:**
- Create: `qml/components/SafetyScenarioOverlay.qml`
- Create: `resources/icons/safety-lab.svg`
- Modify: `src/main.cpp`
- Modify: `qml/screens/DashboardScreen.qml`
- Modify: `CMakeLists.txt`

**Consumes:** the Task 1 service and Task 2 ViewModel contracts.

**Produces:** a Car-only, user-triggered overlay with no new page/state or QML-owned logic.

- [ ] **Step 1: Wire C++ ownership and gating.**

  Add the service/VM sources to the executable and headers to `qt_add_qml_module(... SOURCES ...)`. In `main.cpp`, construct `VehicleModeViewModel` before the new service/VM, expose `SafetyScenario`, and use one C++ lambda to set presentation allowed only when `VehicleMode.vehicleMode() == "car"` and `!ParkingAssist.criticalProximity()`. Connect it to both existing notify signals. When either condition becomes false, the ViewModel stops the lab; it never writes to the existing models.

- [ ] **Step 2: Add the QML module assets.**

  Register `SafetyScenarioOverlay.qml` under `QML_FILES` and `safety-lab.svg` under `RESOURCES`. Add a dimmable top-bar `NeonIconButton` that calls only `SafetyScenario.startDemo()` and is enabled by `SafetyScenario.canStart` and `!ThemeController.isBooting`.

- [ ] **Step 3: Implement the passive overlay.**

  Place `SafetyScenarioOverlay` as an anchored sibling above `CenterHub` inside `centerPanel`, without modifying the existing `StackLayout` or root state. The overlay uses `GlassPanel`, an abstract central threat gate, eight declarative risk segments, C++-supplied text, and only `opacity`, `scale`, and numeric-position Behaviors. Its acknowledgement control calls only `SafetyScenario.acknowledge()`. Do not add a QML Timer, function, conditional block, local mutable state, camera imagery, location/map, or `MultiEffect`.

- [ ] **Step 4: Build and policy-check the integration.**

  Run:

  ```bash
  cmake -S . -B build
  cmake --build build -j2
  rg -n '\\bMath\\.|on[A-Z][A-Za-z]+\\s*:\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml -g '*.qml'
  python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
  ```

  Expected: build exits zero; policy matches are comments only; the project QML linter emits no forbidden-JavaScript finding.

### Task 4: Documentation, regression suite, and final review

**Files:**
- Modify: `AGENTS.md`
- Modify: `docs/architecture.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/ui_ux_guidelines.md`

**Consumes:** verified implementation and commands from Tasks 1–3.

**Produces:** current architecture, test, and task-board documentation that describes the mock-only boundary and verification evidence.

- [ ] **Step 1: Update documentation only after code verification.**

  Add the Safety Scenario service/VM/overlay pipeline, one decision-log row stating that it is mock-only and independent of UART/CenterHub, the `tst_safety_scenario` test target and deterministic `advance()` seam, UI constraints for the simulated disclaimer and abstract visual, and a checked Phase 26 section only after every golden check passes. Do not alter historical claims or unrelated Phase 25 content.

- [ ] **Step 2: Run the complete regression matrix.**

  Run:

  ```bash
  cmake -S . -B build
  cmake --build build -j2
  ctest --test-dir build --output-on-failure
  rg -n '\\bMath\\.|on[A-Z][A-Za-z]+\\s*:\\{|\\b(function|if|for|while|switch|var|let|const)\\b' qml -g '*.qml'
  python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
  QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
  git diff --check
  ```

  Expected: configure/build succeed; all five CTest targets pass; only comment-only scan matches remain; QML lint has no Zero-JS violation; smoke exits due to timeout only and has no QML/runtime errors; diff check is clean.

- [ ] **Step 3: Review the complete change set.**

  Run the project C++ and QML review workflows. Resolve every Critical or Important finding, re-run its covering test, and then repeat the full regression matrix. Do not stage or commit the dirty shared workspace.

## Plan Self-Review

- **Coverage:** Tasks 1–2 create the deterministic mock and C++ ViewModel; Task 3 exposes and renders it; Task 4 updates the project source of truth and verifies all constraints.
- **No placeholders:** Timeline values, text, interfaces, visibility policy, signal policy, test names, and verification commands are specified above.
- **Type consistency:** Service uses `qint64` elapsed time and `qreal` visual values; the ViewModel reflects these types and QML consumes only its `Q_PROPERTY` contract.

# Rear Parking Assist Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a one-sensor, mock-first rear parking-assist panel that automatically replaces Music in the Car CenterHub while reverse is active.

**Architecture:** `MockParkingSensorService` emits validated distance/reverse samples to `ParkingAssistViewModel`; `CenterHubViewModel` converts reverse state to an active CenterHub page. Passive QML keeps both Music and Parking components alive, rendering the C++-owned parking state with the existing Neon Cyberpunk tokens.

**Tech Stack:** C++17, Qt 6.8 Core/QML/Quick/Multimedia, Qt Test, CMake, declarative QML.

## Global Constraints

- QML contains zero imperative JavaScript: no `function`, control flow, mutable QML state, or QML-side formatting/calculation.
- C++17 MVVM: services emit typed samples; ViewModels expose `Q_PROPERTY` values and own all derived state and timers.
- The first release has exactly one rear ultrasonic sensor, no camera, no audio beep, no firmware protocol change, and no QML controls.
- Valid rear distance is an integer from `1` through `250` cm inclusive. Valid samples stale after 1,000 ms while reverse is active.
- Levels are `Unavailable`, `Clear` (151–250), `Caution` (31–150), and `Stop` (1–30); unavailable input never displays a previous distance.
- Music and Parking Assist remain static CenterHub children; only C++ selects page `0` Music or `1` Parking Assist.
- Mock ↔ future UART source replacement must require no QML modification.
- Every behavioral change follows RED → GREEN → refactor and receives focused test evidence before commit.

---

## File Structure

| File | Responsibility |
|---|---|
| `src/services/MockParkingSensorService.{h,cpp}` | Deterministic mock sensor samples and bounded GUI-thread timer. |
| `src/viewmodels/ParkingAssistViewModel.{h,cpp}` | Validates samples, derives parking state, and expires stale input. |
| `src/viewmodels/CenterHubViewModel.{h,cpp}` | Owns the C++ page index derived from parking reverse state. |
| `tests/tst_parking_assist.cpp` | Tests the mock sequence, thresholds, unavailable state, stale timeout, and page handoff. |
| `tests/CMakeLists.txt` | Registers `tst_parking_assist`. |
| `src/main.cpp` | Owns/wires parking services and publishes context properties. |
| `qml/components/ParkingAssistView.qml` | Passive OEM-style rear-sensor panel. |
| `qml/components/CenterHub.qml` | Holds persistent Music and Parking children and binds the C++ page index. |
| `qml/Theme.qml` | Adds centralized parking caution/unavailable colors. |
| `CMakeLists.txt` | Compiles and registers new C++ and QML sources. |
| `README.md`, `docs/*.md` | Records the active architecture, UX, test target, hardware boundary, and task state. |

## Task 1: Parking Domain, Mock Source, and Tests

**Files:**
- Create: `src/services/MockParkingSensorService.h`
- Create: `src/services/MockParkingSensorService.cpp`
- Create: `src/viewmodels/ParkingAssistViewModel.h`
- Create: `src/viewmodels/ParkingAssistViewModel.cpp`
- Create: `src/viewmodels/CenterHubViewModel.h`
- Create: `src/viewmodels/CenterHubViewModel.cpp`
- Create: `tests/tst_parking_assist.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces `MockParkingSensorService::parkingSampleUpdated(int distanceCm, bool reverseActive)`.
- Produces `ParkingAssistViewModel::updateSensorSample(int distanceCm, bool reverseActive)` and `advanceStaleClock(qint64 elapsedMs)`.
- Produces `ParkingAssistViewModel` properties `reverseActive`, `sensorAvailable`, `rearDistanceCm`, `proximityLevel`, `distanceText`, and `statusText`.
- Produces `CenterHubViewModel(ParkingAssistViewModel *)` and `activePage`, where Music is `0` and Parking Assist is `1`.

- [x] **Step 1: Write the failing tests and register their target**

Create `tests/tst_parking_assist.cpp` with this complete test contract:

```cpp
#include <QtTest>
#include <QSignalSpy>
#include "services/MockParkingSensorService.h"
#include "viewmodels/CenterHubViewModel.h"
#include "viewmodels/ParkingAssistViewModel.h"

class TestParkingAssist final : public QObject
{
    Q_OBJECT

private slots:
    void classifiesDistanceThresholds()
    {
        ParkingAssistViewModel viewModel;
        viewModel.updateSensorSample(250, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);
        QCOMPARE(viewModel.statusText(), QStringLiteral("REAR CLEAR"));
        QCOMPARE(viewModel.distanceText(), QStringLiteral("250 CM"));

        viewModel.updateSensorSample(150, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(31, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        viewModel.updateSensorSample(30, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        QCOMPARE(viewModel.statusText(), QStringLiteral("STOP"));
        viewModel.updateSensorSample(1, true);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
    }

    void invalidInputBecomesUnavailable()
    {
        ParkingAssistViewModel viewModel;
        viewModel.updateSensorSample(80, true);
        viewModel.updateSensorSample(0, true);
        QVERIFY(!viewModel.sensorAvailable());
        QCOMPARE(viewModel.rearDistanceCm(), 0);
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
        QCOMPARE(viewModel.distanceText(), QStringLiteral("—"));
        QCOMPARE(viewModel.statusText(), QStringLiteral("SENSOR UNAVAILABLE"));
        viewModel.updateSensorSample(-1, true);
        QVERIFY(!viewModel.sensorAvailable());
        viewModel.updateSensorSample(251, true);
        QVERIFY(!viewModel.sensorAvailable());
    }

    void reverseStateSelectsParkingPage()
    {
        ParkingAssistViewModel parking;
        CenterHubViewModel hub(&parking);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
        parking.updateSensorSample(90, true);
        QCOMPARE(hub.activePage(), CenterHubViewModel::ParkingPage);
        parking.updateSensorSample(0, false);
        QCOMPARE(hub.activePage(), CenterHubViewModel::MusicPage);
    }

    void identicalSampleDoesNotEmitDistanceChanged()
    {
        ParkingAssistViewModel viewModel;
        QSignalSpy spy(&viewModel, &ParkingAssistViewModel::rearDistanceChanged);
        viewModel.updateSensorSample(90, true);
        QCOMPARE(spy.count(), 1);
        viewModel.updateSensorSample(90, true);
        QCOMPARE(spy.count(), 1);
    }

    void validReverseSampleExpiresAfterStaleInterval()
    {
        ParkingAssistViewModel viewModel(1000);
        viewModel.updateSensorSample(90, true);
        viewModel.advanceStaleClock(999);
        QVERIFY(viewModel.sensorAvailable());
        viewModel.advanceStaleClock(1);
        QVERIFY(!viewModel.sensorAvailable());
        QVERIFY(viewModel.reverseActive());
    }

    void mockSequenceCoversAllPresentationStates()
    {
        MockParkingSensorService service;
        ParkingAssistViewModel viewModel;
        QObject::connect(&service, &MockParkingSensorService::parkingSampleUpdated,
                         &viewModel, &ParkingAssistViewModel::updateSensorSample);
        service.start();
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Clear);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Caution);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Stop);
        service.advance(MockParkingSensorService::updateIntervalMs());
        QCOMPARE(viewModel.proximityLevel(), ParkingAssistViewModel::Unavailable);
    }
};

QTEST_MAIN(TestParkingAssist)
#include "tst_parking_assist.moc"
```

Append this target to `tests/CMakeLists.txt`:

```cmake
qt_add_executable(tst_parking_assist
    tst_parking_assist.cpp
    ../src/services/MockParkingSensorService.cpp
    ../src/viewmodels/ParkingAssistViewModel.cpp
    ../src/viewmodels/CenterHubViewModel.cpp
)

target_include_directories(tst_parking_assist PRIVATE ../src)
target_link_libraries(tst_parking_assist PRIVATE Qt6::Core Qt6::Test)
add_test(NAME tst_parking_assist COMMAND tst_parking_assist)
```

- [x] **Step 2: Run the focused target and verify RED**

Run: `cmake -S . -B build && cmake --build build --target tst_parking_assist -j2`

Expected: configuration or compilation fails because the parking service and ViewModels do not exist.

- [x] **Step 3: Implement the minimal C++ domain types**

Implement `ParkingAssistViewModel` with `Q_ENUM(ProximityLevel)` and these declarations:

```cpp
enum ProximityLevel { Unavailable, Clear, Caution, Stop };
Q_PROPERTY(bool reverseActive READ reverseActive NOTIFY reverseActiveChanged)
Q_PROPERTY(bool sensorAvailable READ sensorAvailable NOTIFY sensorAvailableChanged)
Q_PROPERTY(int rearDistanceCm READ rearDistanceCm NOTIFY rearDistanceChanged)
Q_PROPERTY(ProximityLevel proximityLevel READ proximityLevel NOTIFY proximityLevelChanged)
Q_PROPERTY(QString distanceText READ distanceText NOTIFY displayChanged)
Q_PROPERTY(QString statusText READ statusText NOTIFY displayChanged)
void updateSensorSample(int distanceCm, bool reverseActive);
void advanceStaleClock(qint64 elapsedMs);
```

`updateSensorSample` clears distance/availability when reverse is false or the distance is outside `1..250`. For valid reverse samples it stores the distance, restarts a single-shot 1,000-ms `QTimer`, and emits only effective-value signals. The timer calls the same unavailable-state transition without clearing `reverseActive`. `advanceStaleClock` decrements an injected remaining interval and invokes that transition at zero; it exists solely for deterministic tests.

Implement `MockParkingSensorService` as a GUI-thread `QObject` with a single-shot/repeating `QTimer`, a public `start()`, `stop()`, deterministic `advance(qint64 elapsedMs)`, `static constexpr qint64 updateIntervalMs() { return 700; }`, and a cyclic sample sequence `{0,false}`, `{250,true}`, `{150,true}`, `{30,true}`, `{0,true}`, `{0,false}`. `start()` emits the first item immediately; each full interval advances by one item and emits `parkingSampleUpdated`.

Implement `CenterHubViewModel` as a non-owning observer of `ParkingAssistViewModel`. Its `Q_PROPERTY(int activePage READ activePage NOTIFY activePageChanged)` maps `reverseActive == true` to `ParkingPage` and false to `MusicPage`; connect only to `reverseActiveChanged` and suppress duplicate signals.

- [x] **Step 4: Run focused GREEN and refactor only if needed**

Run: `cmake --build build --target tst_parking_assist -j2 && ./build/tests/tst_parking_assist`

Expected: all six tests pass with no failure or warning output.

- [x] **Step 5: Commit the verified C++ domain task**

```bash
git add tests/CMakeLists.txt tests/tst_parking_assist.cpp \
  src/services/MockParkingSensorService.h src/services/MockParkingSensorService.cpp \
  src/viewmodels/ParkingAssistViewModel.h src/viewmodels/ParkingAssistViewModel.cpp \
  src/viewmodels/CenterHubViewModel.h src/viewmodels/CenterHubViewModel.cpp
git commit -m "feat: add parking assist domain model"
```

## Task 2: Runtime Wiring and Passive Parking Panel

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`
- Create: `qml/components/ParkingAssistView.qml`
- Modify: `qml/components/CenterHub.qml`
- Modify: `qml/Theme.qml`

**Interfaces:**
- Consumes Task 1 service and ViewModels.
- Produces context properties `ParkingAssist` and `CenterHubController`.
- Produces a static CenterHub page `0` Music and `1` Parking Assist.

- [x] **Step 1: Preserve Task 1 behavioral evidence before QML integration**

Run: `cmake --build build --target tst_parking_assist -j2 && ./build/tests/tst_parking_assist reverseStateSelectsParkingPage`

Expected: PASS. Task 1 already establishes the only new behavior in this integration task: reverse selects page `1` and leaving reverse selects page `0`. Task 2 adds no C++ decision logic; it binds that tested contract to application ownership and passive QML.

- [x] **Step 2: Wire the application and add passive QML**

In `CMakeLists.txt`, add the three C++ implementation files to `qt_add_executable`, both parking ViewModel files to `qt_add_qml_module(... SOURCES)`, and `qml/components/ParkingAssistView.qml` to `QML_FILES`.

In `src/main.cpp`, construct stack-owned objects before `QQmlApplicationEngine`:

```cpp
MockParkingSensorService parkingSensorService;
ParkingAssistViewModel parkingAssistVm;
CenterHubViewModel centerHubVm(&parkingAssistVm);
QObject::connect(&parkingSensorService,
                 &MockParkingSensorService::parkingSampleUpdated,
                 &parkingAssistVm,
                 &ParkingAssistViewModel::updateSensorSample);
```

Expose `ParkingAssist` and `CenterHubController` as context properties beside the existing ViewModels. After `engine.loadFromModule(...)`, call `parkingSensorService.start()`; it must not depend on serial availability.

In `qml/components/CenterHub.qml`, use `StackLayout` with `currentIndex: CenterHubController.activePage` and two static children, `MusicPlayer {}` followed by `ParkingAssistView {}`. Do not add user gestures or a page indicator.

Create `ParkingAssistView.qml` with a declarative rear-bumper silhouette, three centered proximity bands, C++-provided distance/status text, and this color binding for the active band:

```qml
color: ParkingAssist.proximityLevel === 3
       ? Theme.warningRed
       : ParkingAssist.proximityLevel === 2
         ? Theme.parkingCaution
         : ParkingAssist.proximityLevel === 1
           ? Theme.accentCyan
           : Theme.parkingUnavailable
```

The distance `Text` binds directly to `ParkingAssist.distanceText`; the status `Text` binds directly to `ParkingAssist.statusText`. Both components stay visible while reverse is active. The entire component has `visible: ParkingAssist.reverseActive`, allowing `StackLayout` page selection to remain C++-driven.

Add mutable, animated `Theme.parkingCaution` and `Theme.parkingUnavailable` tokens for day/night color modes. Do not add ad hoc colors to `ParkingAssistView.qml`.

- [x] **Step 3: Run GREEN plus QML policy checks**

Run:

```bash
cmake -S . -B build
cmake --build build --target tst_parking_assist QtStmAutomotiveSimulator -j2
./build/tests/tst_parking_assist
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py qml/components/ParkingAssistView.qml qml/components/CenterHub.qml qml/Theme.qml
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
```

Expected: the focused test passes; the QML linter and module `qmllint` return zero with no executable JavaScript finding.

- [x] **Step 4: Commit the verified runtime and QML task**

```bash
git add CMakeLists.txt src/main.cpp qml/Theme.qml \
  qml/components/CenterHub.qml qml/components/ParkingAssistView.qml \
  tests/tst_parking_assist.cpp
git commit -m "feat: show rear parking assist panel"
```

## Task 3: Active Documentation and Full Verification

**Files:**
- Modify: `AGENTS.md`
- Modify: `README.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`
- Modify: `docs/superpowers/specs/2026-07-29-rear-parking-assist-design.md`

**Interfaces:**
- Consumes the verified implementation from Tasks 1 and 2.
- Produces a single active documentation contract for the mock-first one-sensor parking assist and future STM32 high-level sample boundary.

- [x] **Step 1: Verify the active documentation does not yet describe the feature**

Run: `rg -n 'MockParkingSensorService|tst_parking_assist|one ultrasonic distance' AGENTS.md README.md docs`

Expected: no active documentation contains the new complete parking-assist contract.

- [x] **Step 2: Update active documents and record the completed acceptance contract**

Update the nine listed documents with the same contract: one sensor; valid `1..250` cm; `Clear`/`Caution`/`Stop`/unavailable thresholds; one-second stale timeout; automatic Car CenterHub switch while reverse is active; C++ ownership; mock source now and high-level STM32 UART adapter later; no camera, no audio beep, and no QML changes when hardware replaces mock input. Add the parking domain files and `tst_parking_assist` to README's project tree and deterministic-test table.

Record these checked items in `docs/tasks_board.md` only after the full verification matrix passes:

```markdown
- [x] Document `MockParkingSensorService -> ParkingAssistViewModel -> CenterHubViewModel -> ParkingAssistView`.
- [x] Document that one ultrasonic distance sample is valid only from 1 through 250 cm and becomes unavailable after one second while reverse is active.
- [x] Document that STM32 later supplies high-level distance and reverse state over a deliberately extended UART contract; no video or raw echo timing belongs in QML.
- [x] Document `tst_parking_assist` and the full verification matrix.
```

- [x] **Step 3: Run the full verification matrix**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
QT_QPA_PLATFORM=offscreen timeout 8s ./build/QtStmAutomotiveSimulator
git diff --check
```

Expected: configure/build return zero; all four CTest targets pass; the QML scan has only manually classified comment matches; linter/module `qmllint` return zero; smoke reaches the event loop and ends by timeout code `124` with no QML load/type/binding errors; diff check returns zero.

- [x] **Step 4: Commit documentation and verification evidence**

```bash
git add AGENTS.md README.md docs/architecture.md docs/ui_ux_guidelines.md \
  docs/hardware_integration.md docs/testing_strategy.md docs/tasks_board.md \
  docs/journal.md docs/superpowers/specs/2026-07-29-rear-parking-assist-design.md \
  docs/superpowers/plans/2026-07-29-rear-parking-assist.md
git commit -m "docs: record rear parking assist"
```

## Plan Self-Review

- Spec coverage: Task 1 implements the mock, data validation, thresholds, stale handling, and C++ page ownership; Task 2 integrates the static music/parking CenterHub and theme; Task 3 records the hardware boundary and verifies the application.
- Placeholders: the plan contains no unresolved requirements or deferred implementation markers.
- Type consistency: the typed sample is always `(int distanceCm, bool reverseActive)`; `activePage` always uses `0` Music and `1` Parking; the focused test target is always `tst_parking_assist`.

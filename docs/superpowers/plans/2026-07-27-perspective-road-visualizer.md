# Perspective Road Visualizer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the route-loop neon map with a mock-driven, pseudo-3D single-road visualizer whose C++ wheel-motion boundary can later accept STM32 encoder telemetry without changing QML.

**Architecture:** A deterministic `MockWheelTelemetryService` emits normalized left/right wheel speeds. `RoadMotionViewModel`, implemented as a fixed-size `QAbstractListModel`, validates samples and publishes normalized road-slice geometry; `PerspectiveRoadView.qml` only renders those roles. The old `MapViewModel` and `NeonMapView` remain until the replacement is integrated and smoke-tested, then are removed.

**Tech Stack:** C++17, Qt 6.8+ Core/Qml/Quick/Test, Qt Test, QAbstractListModel, QTimer, Qt Quick Shapes, CMake.

## Global Constraints

- Follow root `AGENTS.md` and all routed architecture, hardware, UI/QML, testing, C++, QML, and CMake standards.
- Absolute Zero JavaScript in QML: no functions, control flow, mutable JS state, block handlers, or `Math.*`.
- New behavior lives in C++ and reaches QML through `Q_PROPERTY`, model roles, or direct invokables.
- Use exactly 24 reusable road rows and a 33 ms mock timer; no per-tick QObject allocation.
- Default scenario targets are `(1.0, 1.0)`, `(0.65, 1.0)`, `(1.0, 1.0)`, `(1.0, 0.65)`, with four 4000 ms stages and a 1000 ms transition.
- Normalize with `wheelTrack = 1.0`, `minimumForwardSpeed = 0.05`, absolute curvature limit `0.75`, maximum accepted delta `100 ms`, and stale timeout `500 ms`.
- The first version supports forward/stopped motion only; reverse and in-place rotation are out of scope.
- Do not treat PWM as measured motion. Future hardware input comes from encoder-derived wheel motion.
- Keep CenterHub/MusicPlayer lifetime and Car/Bike/Scooter behavior unchanged.
- Do not claim physical encoder or STM32 field validation.
- Write a focused failing test before production behavior, observe RED, implement the minimum GREEN change, then review and commit.

---

## File Map

| File | Responsibility |
|---|---|
| `src/services/MockWheelTelemetryService.h` | Mock scenario configuration, lifecycle, deterministic advance API, wheel telemetry signal |
| `src/services/MockWheelTelemetryService.cpp` | Four-stage timing, interpolation, parented 33 ms timer |
| `src/viewmodels/RoadMotionViewModel.h` | Fixed model roles, QML properties, wheel-motion update/reset/stale API |
| `src/viewmodels/RoadMotionViewModel.cpp` | Validation, smoothing, perspective projection, row recycling and model notifications |
| `tests/tst_road_motion.cpp` | Deterministic unit tests for the mock source and road model |
| `tests/CMakeLists.txt` | `tst_road_motion` target and CTest registration |
| `qml/components/PerspectiveRoadView.qml` | Passive single-lane road, fixed marker, perspective slices |
| `qml/components/CenterHub.qml` | Replace the Map page instance with the Road page |
| `qml/Theme.qml` | Reusable road surface/marker visual tokens only |
| `src/main.cpp` | Own, connect, expose, and start the mock road pipeline |
| `CMakeLists.txt` | Register new C++ and QML sources; later remove obsolete map sources |
| `src/viewmodels/MapViewModel.{h,cpp}` | Delete after replacement verification |
| `qml/components/NeonMapView.qml` | Delete after replacement verification |
| `tests/main.cpp` | Remove obsolete route-loop tests after replacement coverage exists |
| `docs/*.md`, `README.md`, `AGENTS.md` | Record verified Phase 18 architecture and status |

---

### Task 1: Deterministic Mock Wheel Telemetry

**Files:**
- Create: `src/services/MockWheelTelemetryService.h`
- Create: `src/services/MockWheelTelemetryService.cpp`
- Create: `tests/tst_road_motion.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `MockWheelTelemetryConfig`
- Produces: `MockWheelTelemetryService(const MockWheelTelemetryConfig &, QObject *)`
- Produces: `void start()`, `void stop()`, `void advance(qint64 elapsedMs)`
- Produces signal: `wheelTelemetryUpdated(double leftWheelSpeed, double rightWheelSpeed, qint64 elapsedMs)`
- Consumes: Qt Core only; no QML and no physical hardware

- [ ] **Step 1: Register the focused test target**

Append this target to `tests/CMakeLists.txt`:

```cmake
qt_add_executable(tst_road_motion
    tst_road_motion.cpp
    ../src/services/MockWheelTelemetryService.cpp
)

target_include_directories(tst_road_motion PRIVATE ../src)

target_link_libraries(tst_road_motion
    PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME tst_road_motion COMMAND tst_road_motion)
```

- [ ] **Step 2: Write the failing mock-scenario tests**

Create `tests/tst_road_motion.cpp` with a `TestRoadMotion` Qt Test class. Use a short injected configuration and `QSignalSpy`; do not wait on a real timer:

```cpp
#include <QtTest>

#include <cmath>

#include "services/MockWheelTelemetryService.h"

class TestRoadMotion final : public QObject
{
    Q_OBJECT

private slots:
    void mockStartsStraight()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(10);

        QCOMPARE(spy.size(), 1);
        const auto sample = spy.takeFirst();
        QCOMPARE(sample.at(0).toDouble(), 1.0);
        QCOMPARE(sample.at(1).toDouble(), 1.0);
        QCOMPARE(sample.at(2).toLongLong(), 10);
    }

    void mockVisitsLeftStraightAndRightStages()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        service.advance(120);
        auto sample = spy.takeLast();
        QVERIFY(sample.at(1).toDouble() > sample.at(0).toDouble());

        service.advance(100);
        sample = spy.takeLast();
        QCOMPARE(sample.at(0).toDouble(), sample.at(1).toDouble());

        service.advance(100);
        sample = spy.takeLast();
        QVERIFY(sample.at(0).toDouble() > sample.at(1).toDouble());
    }

    void mockLoopsWithFiniteSmoothSamples()
    {
        MockWheelTelemetryConfig config;
        config.stageDurationMs = 100;
        config.transitionDurationMs = 20;
        MockWheelTelemetryService service(config);
        QSignalSpy spy(&service, &MockWheelTelemetryService::wheelTelemetryUpdated);

        for (int i = 0; i < 50; ++i) {
            service.advance(10);
        }

        QVERIFY(spy.size() == 50);
        double previousLeft = spy.at(0).at(0).toDouble();
        for (const auto &arguments : spy) {
            const double left = arguments.at(0).toDouble();
            const double right = arguments.at(1).toDouble();
            QVERIFY(std::isfinite(left));
            QVERIFY(std::isfinite(right));
            QVERIFY(std::abs(left - previousLeft) <= 0.175);
            previousLeft = left;
        }
    }
};

QTEST_MAIN(TestRoadMotion)

#include "tst_road_motion.moc"
```

- [ ] **Step 3: Run the focused target and observe RED**

Run:

```bash
cmake -S . -B build
cmake --build build --target tst_road_motion -j2
```

Expected: build fails because `services/MockWheelTelemetryService.h` and the class API do not exist.

- [ ] **Step 4: Add the mock service contract**

Create `src/services/MockWheelTelemetryService.h`:

```cpp
#pragma once

#include <QObject>
#include <QTimer>

struct MockWheelTelemetryConfig
{
    qint64 stageDurationMs = 4000;
    qint64 transitionDurationMs = 1000;
    int timerIntervalMs = 33;
};

class MockWheelTelemetryService final : public QObject
{
    Q_OBJECT

public:
    explicit MockWheelTelemetryService(
        const MockWheelTelemetryConfig &config = {},
        QObject *parent = nullptr);

    void start();
    void stop();
    void advance(qint64 elapsedMs);

signals:
    void wheelTelemetryUpdated(double leftWheelSpeed,
                               double rightWheelSpeed,
                               qint64 elapsedMs);

private:
    struct WheelTarget {
        double left;
        double right;
    };

    WheelTarget targetForStage(int stage) const;
    WheelTarget previousTargetForStage(int stage) const;
    void advanceStageClock(qint64 elapsedMs);

    MockWheelTelemetryConfig m_config;
    QTimer m_timer;
    qint64 m_stageElapsedMs = 0;
    int m_stage = 0;
    bool m_completedCycle = false;
};
```

- [ ] **Step 5: Implement deterministic stage timing and interpolation**

Create `src/services/MockWheelTelemetryService.cpp`. Validate configuration by replacing non-positive durations with defaults, connect the parent-owned value-member timer once, and use clamped linear interpolation:

```cpp
#include "MockWheelTelemetryService.h"

#include <algorithm>
#include <array>
#include <utility>

namespace {
constexpr qint64 kDefaultStageDurationMs = 4000;
constexpr qint64 kDefaultTransitionDurationMs = 1000;
constexpr int kDefaultTimerIntervalMs = 33;
constexpr std::array<std::pair<double, double>, 4> kTargets{{
    {1.0, 1.0},
    {0.65, 1.0},
    {1.0, 1.0},
    {1.0, 0.65}
}};
}
```

Implement the target lookup without exposing the private `WheelTarget`:

```cpp
MockWheelTelemetryService::WheelTarget
MockWheelTelemetryService::targetForStage(int stage) const
{
    const auto &target = kTargets.at(static_cast<std::size_t>(stage));
    return {target.first, target.second};
}

MockWheelTelemetryService::WheelTarget
MockWheelTelemetryService::previousTargetForStage(int stage) const
{
    const int previousStage = stage == 0 ? 3 : stage - 1;
    return targetForStage(previousStage);
}
```

The operative method must follow this exact behavior:

```cpp
void MockWheelTelemetryService::advance(qint64 elapsedMs)
{
    if (elapsedMs <= 0) {
        return;
    }

    advanceStageClock(elapsedMs);
    const WheelTarget target = targetForStage(m_stage);
    const WheelTarget previous =
        m_stage == 0 && !m_completedCycle ? target : previousTargetForStage(m_stage);
    const double ratio = m_config.transitionDurationMs > 0
        ? std::clamp(static_cast<double>(m_stageElapsedMs)
                         / static_cast<double>(m_config.transitionDurationMs),
                     0.0, 1.0)
        : 1.0;

    emit wheelTelemetryUpdated(
        previous.left + (target.left - previous.left) * ratio,
        previous.right + (target.right - previous.right) * ratio,
        elapsedMs);
}
```

`advanceStageClock()` must carry overshoot through a loop, wrap stage 3 to 0, and set `m_completedCycle = true`. `start()` starts the timer at `timerIntervalMs`; the timeout calls `advance(timerIntervalMs)`. `stop()` stops it.

- [ ] **Step 6: Run focused GREEN and the existing suite**

Run:

```bash
cmake --build build --target tst_road_motion -j2
./build/tests/tst_road_motion
ctest --test-dir build --output-on-failure
```

Expected: `tst_road_motion` passes all mock tests and all registered tests pass.

- [ ] **Step 7: Review and commit Task 1**

Check timer ownership, integer conversions, stage-wrap overshoot, and `-Wconversion` output. Then run `git diff --check` and commit:

```bash
git add tests/CMakeLists.txt tests/tst_road_motion.cpp \
    src/services/MockWheelTelemetryService.h \
    src/services/MockWheelTelemetryService.cpp
git commit -m "feat: add mock wheel telemetry scenario"
```

---

### Task 2: Fixed-Size Road Motion Model

**Files:**
- Create: `src/viewmodels/RoadMotionViewModel.h`
- Create: `src/viewmodels/RoadMotionViewModel.cpp`
- Modify: `tests/tst_road_motion.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `updateWheelMotion(double leftWheelSpeed, double rightWheelSpeed, qint64 elapsedMs)`
- Produces: 24-row `QAbstractListModel`
- Produces: read-only `forwardSpeed` and `curvature` properties with effective-value signals
- Produces roles: `leftNearX`, `rightNearX`, `leftFarX`, `rightFarX`, `nearY`, `farY`, `centerNearX`, `centerFarX`, `centerNearY`, `centerFarY`, `centerLineVisible`, `segmentOpacity`, `segmentDepth`
- Produces: `resetRoad()` and a private stale-timeout slot

- [ ] **Step 1: Add the model source to the focused target**

Add `../src/viewmodels/RoadMotionViewModel.cpp` to `tst_road_motion` in `tests/CMakeLists.txt`.

- [ ] **Step 2: Write failing model contract tests**

Extend `tests/tst_road_motion.cpp` with `#include "viewmodels/RoadMotionViewModel.h"` and these focused tests:

```cpp
void roadModelHasStableFiniteRows()
{
    RoadMotionViewModel model;

    QCOMPARE(model.rowCount(), 24);
    for (int row = 0; row < model.rowCount(); ++row) {
        const QModelIndex index = model.index(row, 0);
        QVERIFY(std::isfinite(model.data(index, RoadMotionViewModel::NearYRole).toDouble()));
        QVERIFY(std::isfinite(model.data(index, RoadMotionViewModel::FarYRole).toDouble()));
        QVERIFY(std::isfinite(model.data(index, RoadMotionViewModel::LeftNearXRole).toDouble()));
        QVERIFY(std::isfinite(model.data(index, RoadMotionViewModel::RightNearXRole).toDouble()));
    }
}

void wheelDifferenceControlsApprovedTurnSign()
{
    RoadMotionViewModel model;

    model.updateWheelMotion(0.65, 1.0, 100);
    QVERIFY(model.curvature() > 0.0);

    model.resetRoad();
    model.updateWheelMotion(1.0, 0.65, 100);
    QVERIFY(model.curvature() < 0.0);
}

void equalOrStoppedWheelsDoNotCurve()
{
    RoadMotionViewModel model;

    model.updateWheelMotion(1.0, 1.0, 100);
    QCOMPARE(model.curvature(), 0.0);
    const double depth = model.data(model.index(0, 0),
                                    RoadMotionViewModel::SegmentDepthRole).toDouble();

    model.updateWheelMotion(0.0, 0.0, 100);
    QCOMPARE(model.forwardSpeed(), 0.0);
    QCOMPARE(model.data(model.index(0, 0),
                        RoadMotionViewModel::SegmentDepthRole).toDouble(),
             depth);
}

void invalidSamplesRemainFiniteAndBounded()
{
    RoadMotionViewModel model;
    const double invalid[] = {
        -1.0,
        (std::numeric_limits<double>::infinity)(),
        std::numeric_limits<double>::quiet_NaN()
    };

    for (double value : invalid) {
        model.updateWheelMotion(value, value, 1000);
        QVERIFY(std::isfinite(model.forwardSpeed()));
        QVERIFY(std::isfinite(model.curvature()));
        QVERIFY(std::abs(model.curvature()) <= 0.75);
    }
}

void rowsRecycleWithoutChangingModelSize()
{
    RoadMotionViewModel model;
    const int initialRows = model.rowCount();
    bool wrapped = false;
    double previousDepth = model.data(model.index(23, 0),
                                      RoadMotionViewModel::SegmentDepthRole).toDouble();

    for (int i = 0; i < 100; ++i) {
        model.updateWheelMotion(1.0, 1.0, 100);
        const double depth = model.data(model.index(23, 0),
                                        RoadMotionViewModel::SegmentDepthRole).toDouble();
        wrapped = wrapped || depth < previousDepth;
        previousDepth = depth;
    }

    QCOMPARE(model.rowCount(), initialRows);
    QVERIFY(wrapped);
}
```

Add a signal test proving repeated stopped samples do not re-emit unchanged properties, and a direct private-slot invocation test proving `handleStaleTimeout` sets speed to zero without resetting rows.

- [ ] **Step 3: Run focused RED**

Run:

```bash
cmake --build build --target tst_road_motion -j2
```

Expected: compile fails because `RoadMotionViewModel` and its roles do not exist.

- [ ] **Step 4: Define the model header**

Create `src/viewmodels/RoadMotionViewModel.h`:

```cpp
#pragma once

#include <QAbstractListModel>
#include <QTimer>
#include <QVector>

class RoadMotionViewModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qreal forwardSpeed READ forwardSpeed NOTIFY forwardSpeedChanged)
    Q_PROPERTY(qreal curvature READ curvature NOTIFY curvatureChanged)

public:
    enum Role {
        LeftNearXRole = Qt::UserRole + 1,
        RightNearXRole,
        LeftFarXRole,
        RightFarXRole,
        NearYRole,
        FarYRole,
        CenterNearXRole,
        CenterFarXRole,
        CenterNearYRole,
        CenterFarYRole,
        CenterLineVisibleRole,
        SegmentOpacityRole,
        SegmentDepthRole
    };
    Q_ENUM(Role)

    explicit RoadMotionViewModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    qreal forwardSpeed() const;
    qreal curvature() const;

public slots:
    void updateWheelMotion(double leftWheelSpeed,
                           double rightWheelSpeed,
                           qint64 elapsedMs);
    void resetRoad();

signals:
    void forwardSpeedChanged();
    void curvatureChanged();

private slots:
    void handleStaleTimeout();

private:
    struct Segment {
        double depth = 0.0;
        quint64 generation = 0;
    };

    void updatePublishedMotion(double speed, double curvature);
    void advanceSegments(double elapsedSeconds);
    QVariant roleData(const Segment &segment, int row, int role) const;
    double projectedY(double depth) const;
    double projectedCenter(double depth) const;
    double projectedHalfWidth(double depth) const;

    QVector<Segment> m_segments;
    QTimer m_staleTimer;
    qreal m_forwardSpeed = 0.0;
    qreal m_curvature = 0.0;
};
```

- [ ] **Step 5: Implement validation, smoothing, projection, and recycling**

Create `src/viewmodels/RoadMotionViewModel.cpp` with these constants and rules:

```cpp
namespace {
constexpr int kSegmentCount = 24;
constexpr qint64 kMaximumElapsedMs = 100;
constexpr int kStaleTimeoutMs = 500;
constexpr double kWheelTrack = 1.0;
constexpr double kMinimumForwardSpeed = 0.05;
constexpr double kMaximumCurvature = 0.75;
constexpr double kScrollScale = 0.45;
constexpr double kCurvatureResponsePerSecond = 4.0;
constexpr double kHorizonY = 0.14;
constexpr double kRoadHeight = 0.82;
}
```

Constructor behavior:

```cpp
m_segments.reserve(kSegmentCount);
for (int row = 0; row < kSegmentCount; ++row) {
    m_segments.push_back({
        static_cast<double>(row) / static_cast<double>(kSegmentCount),
        0
    });
}
m_staleTimer.setSingleShot(true);
m_staleTimer.setInterval(kStaleTimeoutMs);
connect(&m_staleTimer, &QTimer::timeout,
        this, &RoadMotionViewModel::handleStaleTimeout);
```

`updateWheelMotion()` must:

1. Ignore `elapsedMs <= 0`.
2. Neutralize non-finite wheel values to zero and clamp negative values to zero.
3. Clamp elapsed time to 100 ms.
4. Compute speed average.
5. Compute target curvature only above minimum speed.
6. Clamp target to `[-0.75, 0.75]`.
7. Smooth with `alpha = min(1.0, elapsedSeconds * 4.0)`.
8. Advance segment depths only if effective speed is positive.
9. Restart the stale timer on every accepted sample.
10. Emit `dataChanged(index(0,0), index(23,0), allGeometryRoles)` without resetting the model.

Projection must use finite normalized values:

```cpp
double RoadMotionViewModel::projectedY(double depth) const
{
    return kHorizonY + std::pow(std::clamp(depth, 0.0, 1.0), 1.7) * kRoadHeight;
}

double RoadMotionViewModel::projectedCenter(double depth) const
{
    const double d = std::clamp(depth, 0.0, 1.0);
    return std::clamp(0.5 - static_cast<double>(m_curvature) * d * d * 0.30,
                      -0.10, 1.10);
}

double RoadMotionViewModel::projectedHalfWidth(double depth) const
{
    const double d = std::clamp(depth, 0.0, 1.0);
    return 0.04 + d * d * 0.44;
}
```

For each row, use `farDepth = segment.depth` and
`nearDepth = min(1.0, segment.depth + 1.0 / 24.0)`. When advancing makes
`depth >= 1.0`, subtract `1.0` in a loop and increment `generation`. Derive
`centerLineVisible` from `(row + generation) % 2 == 0`.

`handleStaleTimeout()` sets only effective speed to zero and emits
`forwardSpeedChanged` when needed. It leaves geometry and curvature stable.

- [ ] **Step 6: Run focused GREEN, full build, and C++ review**

Run:

```bash
cmake --build build --target tst_road_motion -j2
./build/tests/tst_road_motion
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Then run the repository `qt-cpp-review` workflow against the new service,
ViewModel, test, and mandatory timer ownership paths. Fix in-scope
high-confidence findings and rerun the focused/full tests.

- [ ] **Step 7: Commit Task 2**

```bash
git add src/viewmodels/RoadMotionViewModel.h \
    src/viewmodels/RoadMotionViewModel.cpp \
    tests/tst_road_motion.cpp tests/CMakeLists.txt
git commit -m "feat: model perspective road motion"
```

---

### Task 3: Application Wiring and Passive Perspective Road View

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`
- Create: `qml/components/PerspectiveRoadView.qml`
- Modify: `qml/components/CenterHub.qml`
- Modify: `qml/Theme.qml`

**Interfaces:**
- Consumes: `MockWheelTelemetryService::wheelTelemetryUpdated(...)`
- Consumes: `RoadMotionViewModel::updateWheelMotion(...)`
- Consumes QML model: context property `RoadMotion`
- Keeps temporarily: old `MapModel`, `MapViewModel`, and `NeonMapView.qml` until Task 4

- [ ] **Step 1: Add new production sources while retaining the old map**

In the `qt_add_executable` sources, add:

```cmake
src/services/MockWheelTelemetryService.cpp
```

In `qt_add_qml_module(... QML_FILES ...)`, add:

```cmake
qml/components/PerspectiveRoadView.qml
```

In `qt_add_qml_module(... SOURCES ...)`, add:

```cmake
src/viewmodels/RoadMotionViewModel.h
src/viewmodels/RoadMotionViewModel.cpp
```

- [ ] **Step 2: Wire the new C++ pipeline**

Add includes to `src/main.cpp`:

```cpp
#include "services/MockWheelTelemetryService.h"
#include "viewmodels/RoadMotionViewModel.h"
```

Construct and connect the road pipeline before loading QML:

```cpp
RoadMotionViewModel roadMotionVm;
MockWheelTelemetryService mockWheelTelemetry;
QObject::connect(
    &mockWheelTelemetry,
    &MockWheelTelemetryService::wheelTelemetryUpdated,
    &roadMotionVm,
    &RoadMotionViewModel::updateWheelMotion);
```

Expose:

```cpp
engine.rootContext()->setContextProperty("RoadMotion", &roadMotionVm);
```

Call `mockWheelTelemetry.start()` only after the connection is installed and
before `app.exec()`. Keep `MapModel` and trip-to-map wiring in this task so the
old page can still load while QML integration is being completed.

- [ ] **Step 3: Add reusable road visual tokens**

Add themed properties to `qml/Theme.qml`:

```qml
property color roadSurface: ThemeController.isNight ? "#CC111A24" : "#DDE5EBF0"
property color roadLaneMarker: ThemeController.isNight ? "#E6FFFFFF" : "#CC1A2530"
property color roadHorizonGlow: ThemeController.isNight ? "#4066FCF1" : "#3000857C"

Behavior on roadSurface { ColorAnimation { duration: durationTheme } }
Behavior on roadLaneMarker { ColorAnimation { duration: durationTheme } }
Behavior on roadHorizonGlow { ColorAnimation { duration: durationTheme } }
```

- [ ] **Step 4: Create the passive QML road**

Create `qml/components/PerspectiveRoadView.qml`. Use only imports
`QtQuick`, `QtQuick.Shapes`, and `com.showcase`. The component must have:

```qml
Rectangle {
    id: root
    color: Theme.glassPanelBase
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd
    clip: true

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.45
        height: parent.height * 0.18
        radius: height / 2
        color: Theme.roadHorizonGlow
        opacity: 0.75
    }

    Repeater {
        model: RoadMotion

        Shape {
            required property real leftNearX
            required property real rightNearX
            required property real leftFarX
            required property real rightFarX
            required property real nearY
            required property real farY
            required property real centerNearX
            required property real centerFarX
            required property real centerNearY
            required property real centerFarY
            required property bool centerLineVisible
            required property real segmentOpacity
            required property real segmentDepth

            anchors.fill: parent
            opacity: segmentOpacity
            z: segmentDepth

            ShapePath {
                strokeWidth: 0
                fillColor: Theme.roadSurface
                startX: leftFarX * root.width
                startY: farY * root.height
                PathLine { x: rightFarX * root.width; y: farY * root.height }
                PathLine { x: rightNearX * root.width; y: nearY * root.height }
                PathLine { x: leftNearX * root.width; y: nearY * root.height }
                PathLine { x: leftFarX * root.width; y: farY * root.height }
            }

            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 2
                fillColor: "transparent"
                startX: leftFarX * root.width
                startY: farY * root.height
                PathLine { x: leftNearX * root.width; y: nearY * root.height }
            }

            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 2
                fillColor: "transparent"
                startX: rightFarX * root.width
                startY: farY * root.height
                PathLine { x: rightNearX * root.width; y: nearY * root.height }
            }

            ShapePath {
                strokeColor: centerLineVisible ? Theme.roadLaneMarker : "transparent"
                strokeWidth: 2
                fillColor: "transparent"
                startX: centerFarX * root.width
                startY: centerFarY * root.height
                PathLine {
                    x: centerNearX * root.width
                    y: centerNearY * root.height
                }
            }
        }
    }

    Shape {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.spaceXl
        width: 28
        height: 38
        z: 100

        ShapePath {
            strokeColor: Theme.accentCyan
            strokeWidth: 2
            fillColor: Theme.textPrimary
            startX: 14
            startY: 0
            PathLine { x: 28; y: 38 }
            PathLine { x: 14; y: 29 }
            PathLine { x: 0; y: 38 }
            PathLine { x: 14; y: 0 }
        }
    }
}
```

Do not add a mock-status label in Phase 18; it is optional in the spec and
would add visual noise without helping the approved interaction.

- [ ] **Step 5: Switch CenterHub to the replacement page**

In `qml/components/CenterHub.qml`, replace:

```qml
NeonMapView {}
```

with:

```qml
PerspectiveRoadView {}
```

Update the component comment from “Music ⇄ Map” to “Music ⇄ Road”. Do not
change SwipeView or PageIndicator behavior.

- [ ] **Step 6: Verify QML policy and runtime**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator
```

Expected:

- configure/build exit 0;
- all CTest targets pass;
- Zero-JS scan has zero executable matches;
- module `qmllint` exits 0;
- smoke exits 124 after reaching the event loop, with no QML load, type,
  binding, or recursive-effect errors.

Visually inspect the Car CenterHub: marker fixed near the bottom, one road
narrowing to the horizon, continuous scroll, smooth left/right bends, and
Music/Road swipe still responsive.

- [ ] **Step 7: Run formal QML and C++ reviews**

Run `qt-qml-review` over all changed QML and `qt-cpp-review` over the mock
timer, road model, and `main.cpp` wiring. Resolve every high-confidence
in-scope finding and repeat Step 6.

- [ ] **Step 8: Commit Task 3**

```bash
git add CMakeLists.txt src/main.cpp qml/Theme.qml \
    qml/components/CenterHub.qml \
    qml/components/PerspectiveRoadView.qml
git commit -m "feat: render mock-driven perspective road"
```

---

### Task 4: Remove the Obsolete Route-Loop Map

**Files:**
- Delete: `src/viewmodels/MapViewModel.h`
- Delete: `src/viewmodels/MapViewModel.cpp`
- Delete: `qml/components/NeonMapView.qml`
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/main.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Removes: `MapModel`, `MapViewModel::routeProgress`, and trip-to-map wiring
- Preserves: `TripComputerViewModel` and `TripComputer` QML context property
- Preserves: `RoadMotion` as the only road visualization model

- [ ] **Step 1: Prove the replacement no longer references old types**

Run:

```bash
rg -n 'MapModel|MapViewModel|NeonMapView|routeProgress' \
    src qml tests CMakeLists.txt
```

Expected before cleanup: matches exist only in the obsolete C++/QML files,
their CMake registrations, `main.cpp` old wiring, and old `tests/main.cpp`
route-progress tests. If a new road file references them, stop and fix Task 3.

- [ ] **Step 2: Remove old production wiring and registrations**

Delete from `src/main.cpp`:

```cpp
#include "viewmodels/MapViewModel.h"
MapViewModel mapVm;
QObject::connect(&tripVm, &TripComputerViewModel::tripChanged, &mapVm,
                 [&tripVm, &mapVm]() {
                     mapVm.updateDistance(tripVm.odometerKm());
                 });
engine.rootContext()->setContextProperty("MapModel", &mapVm);
```

Remove `qml/components/NeonMapView.qml` and
`src/viewmodels/MapViewModel.{h,cpp}` from `CMakeLists.txt`, then delete those
three files.

- [ ] **Step 3: Remove obsolete tests**

Delete the `MapViewModel.h` include and all `testMap*` methods from
`tests/main.cpp`. Remove `../src/viewmodels/MapViewModel.cpp` from the
`tst_viewmodels` target in `tests/CMakeLists.txt`.

- [ ] **Step 4: Verify no obsolete runtime reference remains**

Run:

```bash
rg -n 'MapModel|MapViewModel|NeonMapView|routeProgress' \
    src qml tests CMakeLists.txt
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator
```

Expected: `rg` returns no matches, build succeeds, all tests pass, and smoke
reaches the event loop without QML errors.

- [ ] **Step 5: Commit Task 4**

```bash
git add -A src/viewmodels/MapViewModel.h \
    src/viewmodels/MapViewModel.cpp qml/components/NeonMapView.qml \
    src/main.cpp CMakeLists.txt tests/main.cpp tests/CMakeLists.txt
git commit -m "refactor: retire route-loop map"
```

---

### Task 5: Synchronize Phase 18 Documentation and Run the Final Matrix

**Files:**
- Modify: `AGENTS.md`
- Modify: `README.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`
- Modify: `docs/hardware_integration.md` only to clarify the future encoder boundary; do not claim implementation

**Interfaces:**
- Documents: `MockWheelTelemetryService -> RoadMotionViewModel -> PerspectiveRoadView`
- Records: mock-only verification and explicit pending encoder/STM32 validation
- Removes active-document claims about route-loop `MapModel.routeProgress`

- [ ] **Step 1: Update active architecture and UI truth**

Apply these exact content changes:

- `AGENTS.md`: add an immutable decision for **Encoder-Compatible Perspective
  Road**: the fixed vehicle/moving road consumes left/right wheel telemetry;
  mock is implemented first and PWM is not treated as measured motion.
- `docs/architecture.md`: replace `Trip -> MapViewModel -> QML` with
  `MockWheelTelemetryService -> RoadMotionViewModel -> PerspectiveRoadView`;
  document the fixed 24-row normalized geometry model and future
  source-swap boundary.
- `docs/ui_ux_guidelines.md`: replace `NeonMapView`/route-marker wording with
  the approved fixed-marker, single-lane perspective road and state that all
  geometry comes from C++.
- `docs/testing_strategy.md`: add `tst_road_motion` responsibilities and the
  required scenario/model boundary cases.
- `docs/hardware_integration.md`: state that encoder-derived left/right wheel
  motion is the future road input and PWM comparison is not accepted as
  odometry; leave physical validation pending.

- [ ] **Step 2: Record verified project status**

After, and only after, the verification matrix passes:

- Add Phase 18 to `docs/tasks_board.md` with checked implementation,
  integration, TDD, Zero-JS/review, and final-matrix items; retain an unchecked
  physical encoder integration item.
- Add a dated Phase 18 decision entry to `docs/journal.md` containing the
  source boundary, geometry ownership, mock scenario, exact verification
  results, review disposition, and remaining hardware limitation.
- Update the README feature/phase summary from a general neon map to the
  pseudo-3D single-road visualizer.

- [ ] **Step 3: Run the complete final verification matrix**

Run fresh:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
rg -n '\bMath\.|on[A-Z][A-Za-z]+\s*:\s*\{|\b(function|if|for|while|switch|var|let|const)\b' qml -g '*.qml'
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py \
    $(rg --files qml -g '*.qml')
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
git diff --check
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator
```

Classify scan/review matches. Expected:

- configure/build exit 0 without compiler warnings;
- all four CTest targets pass;
- no executable QML JavaScript;
- no high-confidence in-scope QML/C++ review finding;
- module `qmllint` exits 0;
- diff hygiene clean;
- smoke reaches the event loop and exits 124 only because of `timeout`.

- [ ] **Step 4: Re-review documentation against the repository**

Run:

```bash
rg -n 'MapModel|MapViewModel|NeonMapView|routeProgress|general neon map' \
    AGENTS.md README.md docs .agents --glob '*.md'
rg -n 'encoder|PWM|PerspectiveRoadView|RoadMotionViewModel' \
    AGENTS.md README.md docs .agents --glob '*.md'
```

Historical Phase 16 spec/plan references may remain when clearly dated.
Active docs must contain no stale route-loop claim and must not claim physical
encoder support.

- [ ] **Step 5: Request final branch review and resolve findings**

Prepare a review package from the merge base through HEAD. Request a final
read-only review focused on:

- exact spec coverage;
- C++ timer/model ownership and finite arithmetic;
- stable QAbstractListModel roles/notifications;
- Zero-JS QML and geometry ownership;
- CenterHub/Music/vehicle-mode regressions;
- removal of obsolete map runtime wiring;
- active Markdown truth.

If findings are valid, use `receiving-code-review`, add RED regression tests
for behavioral fixes, implement, rerun Step 3, and request a bounded re-review.

- [ ] **Step 6: Commit Task 5**

```bash
git add AGENTS.md README.md docs/architecture.md \
    docs/ui_ux_guidelines.md docs/testing_strategy.md \
    docs/tasks_board.md docs/journal.md docs/hardware_integration.md
git commit -m "docs: record perspective road verification"
```

- [ ] **Step 7: Finish the development branch**

Use `verification-before-completion`, then
`finishing-a-development-branch`. Do not push, merge, or delete the branch
without the user's explicit choice at that final gate.

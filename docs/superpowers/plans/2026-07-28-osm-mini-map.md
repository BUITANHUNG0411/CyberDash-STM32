# OSM Follow Mini-Map Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
> **Status:** Completed and verified in Phase 20; retained as the implementation record.

**Goal:** Provide a rectangular, north-up OSM mini-map whose C++-driven marker follows a deterministic mock route and whose source can later be replaced by GNSS or localized encoder hardware.

**Architecture:** `MockPositionSource` subclasses `QGeoPositionInfoSource` and emits deterministic `QGeoPositionInfo` updates. `MapViewModel` owns validated position, bearing, viewport pan/zoom, follow timeout, and Web-Mercator math. Passive QML renders a raw Qt Location `Map`, forwards gesture deltas to C++, and places one `MapQuickItem` marker.

**Tech Stack:** C++17, Qt 6.8+ Core/Positioning/Location/QML/Quick, Qt Test, Qt Location OSM plugin, CMake.

## Global Constraints

- Zero imperative JavaScript in repository QML; handlers contain one direct C++ invokable call only.
- Map bearing is always `0`; only the marker rotates using C++ `bearingDegrees`.
- Public OSM use keeps copyrights visible, sets an identifying User-Agent, disables prefetching, and never implements bulk/offline download.
- CTest never depends on network tile delivery.
- Preserve `SimulatorService`, `SerialService`, `TelemetryMapper`, dashboard/trip ViewModels, MusicPlayer lifetime, UART fallback, and vehicle-mode layouts.

---

### Task 1: Position source and navigation ViewModel with TDD

**Files:**
- Create: `tests/tst_map_navigation.cpp`
- Create: `src/services/MockPositionSource.h`
- Create: `src/services/MockPositionSource.cpp`
- Create: `src/viewmodels/MapViewModel.h`
- Create: `src/viewmodels/MapViewModel.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `MockPositionSource : QGeoPositionInfoSource`, deterministic `advance(qint64 elapsedMs)`, and standard `positionUpdated(const QGeoPositionInfo &)`.
- Produces: `MapViewModel::setPositionSource(QGeoPositionInfoSource *)`, `position`, `bearingDegrees`, `viewportCenter`, `zoomLevel`, `followEnabled`, `followLabel`.
- Produces invokables: `panByPixels(qreal dx, qreal dy, qreal viewportWidth, qreal viewportHeight)`, `zoomByWheelDelta(qreal angleDeltaY, qreal pixelDeltaY)`, `zoomByPinchScale(qreal scaleDelta)`, `resumeFollow()`, and deterministic `advanceFollowClock(qint64 elapsedMs)`.

- [x] **Step 1: Write the focused RED test target**

Add `tst_map_navigation` linked to `Qt6::Core`, `Qt6::Positioning`, and `Qt6::Test`. Tests must cover these exact behaviors:

```cpp
void sourceInterpolatesWithinSegment();
void sourceBearingMatchesActiveSegment();
void sourceWrapsAfterCompleteRoute();
void sourceRejectsInvalidRouteAndElapsedTime();
void sourceLifecycleIsIdempotent();
void viewModelFollowsInjectedSource();
void replacingSourceDisconnectsOldSource();
void panSuspendsFollowAndMovesViewport();
void zoomGesturesClampAndRestartFollowDeadline();
void followResumesAtInjectedTimeout();
void invalidFixDoesNotChangeViewModel();
```

- [x] **Step 2: Run RED**

```bash
cmake -S . -B build
cmake --build build --target tst_map_navigation -j2
```

Expected: build fails because `MockPositionSource` and `MapViewModel` do not exist.

- [x] **Step 3: Implement `MockPositionSource`**

The public shape must include:

```cpp
struct MockPositionConfig {
    QVector<QGeoCoordinate> route;
    qreal speedMetersPerSecond = 8.0;
    qint64 tickIntervalMs = 100;
    qint64 maximumAdvanceMs = 1000;
};

class MockPositionSource final : public QGeoPositionInfoSource {
    Q_OBJECT
public:
    explicit MockPositionSource(const MockPositionConfig &config = {},
                                QObject *parent = nullptr);
    PositioningMethods supportedPositioningMethods() const override;
    int minimumUpdateInterval() const override;
    Error error() const override;
    void advance(qint64 elapsedMs);
public slots:
    void startUpdates() override;
    void stopUpdates() override;
    void requestUpdate(int timeout = 0) override;
};
```

Use a closed default route with at least four valid coordinates. Advance by physical meters using `distanceTo`, `azimuthTo`, and `atDistanceAndAzimuth`; attach `QGeoPositionInfo::Direction`; cap elapsed time; never emit invalid/non-finite output.

- [x] **Step 4: Implement `MapViewModel`**

The QML surface must include:

```cpp
Q_PROPERTY(QGeoCoordinate position READ position NOTIFY positionChanged)
Q_PROPERTY(qreal bearingDegrees READ bearingDegrees NOTIFY bearingDegreesChanged)
Q_PROPERTY(QGeoCoordinate viewportCenter READ viewportCenter NOTIFY viewportCenterChanged)
Q_PROPERTY(qreal zoomLevel READ zoomLevel NOTIFY zoomLevelChanged)
Q_PROPERTY(bool followEnabled READ followEnabled NOTIFY followEnabledChanged)
Q_PROPERTY(QString followLabel READ followLabel NOTIFY followEnabledChanged)
```

Normalize bearings into `[0, 360)`. Use default zoom `16.5`, bounds `3.0..19.0`, follow timeout `4000 ms`, latitude clamp `±85.05112878`, longitude wrapping, and a 256-pixel Web-Mercator tile basis. Ignore invalid fixes and effective no-change updates. A replaced or destroyed position source must not update the ViewModel.

- [x] **Step 5: Run GREEN**

```bash
cmake --build build --target tst_map_navigation -j2
./build/tests/tst_map_navigation
```

Expected: all navigation cases pass with no warning.

- [x] **Step 6: Review Task 1**

Run focused C++ review on the four new production files and resolve Critical/Important findings before continuing.

---

### Task 2: OSM QML integration

**Files:**
- Create: `qml/components/OsmMiniMapView.qml`
- Modify: `qml/components/CenterHub.qml`
- Modify: `qml/Theme.qml`
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `MapViewModel` properties and invokables from Task 1.
- Produces: context property `MapModel`, CenterHub Music + OSM pages, and Qt Location dependency wiring.

- [x] **Step 1: Write the QML/static RED expectations**

Before adding the view, confirm these checks fail:

```bash
test -f qml/components/OsmMiniMapView.qml
rg -n 'QtLocation|MapQuickItem|MapModel' qml/components/OsmMiniMapView.qml
```

- [x] **Step 2: Update CMake and main wiring**

Add `Location` and `Positioning` to `find_package`; link `Qt6::Location` and `Qt6::Positioning`. Register `MockPositionSource.cpp`, `MapViewModel.{h,cpp}`, and `OsmMiniMapView.qml`.

In `main.cpp`, stack-own:

```cpp
MockPositionSource mockPositionSource;
MapViewModel mapVm;
mapVm.setPositionSource(&mockPositionSource);
```

Expose it as `MapModel`, start the source after QML loads, and preserve all serial/simulator/trip connections unchanged.

- [x] **Step 3: Add passive `OsmMiniMapView.qml`**

Use unversioned imports:

```qml
import QtQuick
import QtQuick.Shapes
import QtLocation
import QtPositioning
import com.showcase
```

The OSM plugin must set:

```qml
PluginParameter { name: "osm.useragent"; value: "QtStmAutomotiveSimulator/1.0" }
PluginParameter { name: "osm.mapping.prefetching_style"; value: "NoPrefetching" }
```

Use raw `Map` with `center: MapModel.viewportCenter`, `zoomLevel: MapModel.zoomLevel`, `bearing: 0`, `tilt: 0`, `copyrightsVisible: true`, and dark fallback color. Add `MapQuickItem` at `MapModel.position`; its compact vector source rotates by `MapModel.bearingDegrees`.

Gesture handlers contain only direct calls:

```qml
onTranslationChanged: MapModel.panByPixels(delta.x, delta.y, map.width, map.height)
onWheel: MapModel.zoomByWheelDelta(event.angleDelta.y, event.pixelDelta.y)
onScaleChanged: MapModel.zoomByPinchScale(delta)
```

The status pill binds to `MapModel.followLabel`. Do not add route lines, destination controls, search, routing, offline download, or QML calculations.

- [x] **Step 4: Replace the CenterHub page and remove theme residue**

Add `OsmMiniMapView {}` as the second static `SwipeView` page while keeping both children static. Add only the minimal map frame/marker/status tokens required by the component.

- [x] **Step 5: Run integration GREEN**

Run:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
```

Expected: configure/build pass; four registered targets pass (`tst_viewmodels`, `tst_music_playback`, `tst_serial_pipeline`, `tst_map_navigation`); QML checks report no executable JavaScript or type error.

- [x] **Step 6: Review Task 2**

Run both project `qt-cpp-review` and `qt-qml-review`; resolve Critical/Important findings before continuing.

---

### Task 3: Synchronize active documentation

**Files:**
- Modify: `AGENTS.md`
- Modify: `README.md`
- Modify: `docs/architecture.md`
- Modify: `docs/ui_ux_guidelines.md`
- Modify: `docs/hardware_integration.md`
- Modify: `docs/testing_strategy.md`
- Modify: `docs/tasks_board.md`
- Modify: `docs/journal.md`

**Interfaces:**
- Consumes: verified runtime and test names from Tasks 1–2.
- Produces: one canonical Phase 20 OSM contract.

- [x] **Step 1: Synchronize active guides**

Document:

- `MockPositionSource -> MapViewModel -> OsmMiniMapView`;
- Qt Location/Positioning build modules;
- north-up marker-bearing contract;
- follow/explore behavior and four-second timeout;
- OSM attribution/User-Agent/cache/no-prefetch policy;
- GNSS or encoder localization as future source adapters;
- `tst_map_navigation` as the fourth registered target;
- Phase 20 as the current map phase.

- [x] **Step 2: Residue scan**

Run the repository residue scan prescribed by the Task 3 brief.

Expected: zero results.

- [x] **Step 3: Full verification**

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
python3 .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py all qml
/usr/lib/qt6/bin/qmllint --module com.showcase -I build
git diff --check
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/QtStmAutomotiveSimulator
```

Expected: build succeeds; all tests pass; Zero-JS/QML checks pass; diff is clean; smoke reaches the event loop without QML load/type/binding errors. Network tile warnings do not establish a software failure if the OSM provider is unreachable.


**Handoff:**

Do not commit unless the user explicitly authorizes it.

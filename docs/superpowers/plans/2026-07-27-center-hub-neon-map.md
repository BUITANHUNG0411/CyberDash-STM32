# Center Hub (Music ⇄ Map) + Neon Map Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn the Car-mode center panel into a swipeable SwipeView hub (Music Player ⇄ stylized Neon Map) whose map marker advances along a route using real odometer distance.

**Architecture:** New `MapViewModel` (context property `MapModel`) converts `TripComputerViewModel::odometerKm` into a wrapping `routeProgress` (0..1). New `NeonMapView.qml` draws a Shapes-based neon map; a `PathInterpolator` bound to `MapModel.routeProgress` positions/rotates the marker. New `CenterHub.qml` wraps `MusicPlayer` + `NeonMapView` in a `SwipeView` (static children — never destroyed) with a custom `PageIndicator`. DashboardScreen's Phase-14 states retarget `musicPlayer` → `centerHub`.

**Tech Stack:** Qt 6.8 (Quick, Quick Controls Basic — first use, Shapes), CMake, QtTest.

**Spec:** `docs/superpowers/specs/2026-07-27-center-hub-neon-map-design.md`

## Global Constraints

- Zero-JS QML: no `function/if(/for(/while(/switch(/var/let/const/toFixed/toUpperCase`. Ternaries, `Math.min`/`Math.max` inside bindings (existing precedent in MusicPlayer.qml), States/Transitions, Behavior, PathInterpolator are allowed.
- MVVM: new behavior in C++; QML unchanged when swapping Simulator ↔ Serial.
- C++17, `-Wall -Wextra -Wconversion -Wsign-conversion` must stay clean.
- MultiEffect Sibling Source rule: effect source must never contain the effect.
- Work on branch `feature/center-hub-map`; commits per task on the branch; **NO merge to `main` until the user confirms** (then single squash commit + push).
- Hub exists ONLY in Car mode; scooter (RangeTripCard) and bike (empty center) behavior unchanged.

---

### Task 1: `MapViewModel` (TDD)

**Files:**
- Create: `src/viewmodels/MapViewModel.h`, `src/viewmodels/MapViewModel.cpp`
- Modify: `tests/main.cpp` (include + 3 tests), `tests/CMakeLists.txt`, root `CMakeLists.txt` (SOURCES)

**Interfaces:**
- Produces: `MapViewModel(double routeLengthKm = 2.0, QObject *parent = nullptr)`; `qreal routeProgress() const`; public slot `void updateDistance(double odometerKm)`; signal `void routeProgressChanged()`.

- [ ] **Step 1: failing tests** — append to `tests/main.cpp` (add `#include "viewmodels/MapViewModel.h"` at top):

```cpp
    // --- MapViewModel (Phase 16) ---

    void testMapDefaults() {
        MapViewModel map;
        QCOMPARE(map.routeProgress(), 0.0);
    }

    void testMapProgressAdvances() {
        MapViewModel map(2.0);
        QSignalSpy spy(&map, &MapViewModel::routeProgressChanged);

        map.updateDistance(0.5); // 0.5 km on a 2.0 km loop -> 0.25
        QCOMPARE(map.routeProgress(), 0.25);
        QCOMPARE(spy.count(), 1);

        map.updateDistance(0.5); // same value -> no re-emit
        QCOMPARE(spy.count(), 1);
    }

    void testMapProgressWraps() {
        MapViewModel map(2.0);
        map.updateDistance(2.5); // wraps past 1.0 -> 0.25
        QCOMPARE(map.routeProgress(), 0.25);
    }
```

Add `../src/viewmodels/MapViewModel.cpp` to `tst_viewmodels` in `tests/CMakeLists.txt`.

- [ ] **Step 2: verify RED** — `cmake -B build` fails (missing header).

- [ ] **Step 3: implement** — `src/viewmodels/MapViewModel.h`:

```cpp
#pragma once

#include <QObject>

class MapViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal routeProgress READ routeProgress NOTIFY routeProgressChanged)

public:
    // routeLengthKm: length of the map's route loop; injectable for tests.
    explicit MapViewModel(double routeLengthKm = 2.0, QObject *parent = nullptr);

    qreal routeProgress() const;

public slots:
    void updateDistance(double odometerKm);

signals:
    void routeProgressChanged();

private:
    double m_routeLengthKm;
    qreal m_routeProgress = 0.0;
};
```

`src/viewmodels/MapViewModel.cpp`:

```cpp
#include "MapViewModel.h"

#include <cmath>

MapViewModel::MapViewModel(double routeLengthKm, QObject *parent)
    : QObject(parent), m_routeLengthKm(routeLengthKm) {}

qreal MapViewModel::routeProgress() const { return m_routeProgress; }

void MapViewModel::updateDistance(double odometerKm)
{
    const qreal progress = std::fmod(odometerKm / m_routeLengthKm, 1.0);
    if (qFuzzyCompare(m_routeProgress + 1.0, progress + 1.0)) {
        return;
    }
    m_routeProgress = progress;
    emit routeProgressChanged();
}
```

Add both files to root `qt_add_qml_module(... SOURCES ...)`.

- [ ] **Step 4: verify GREEN** — build 0 warnings; `ctest`: 31 viewmodel tests pass.
- [ ] **Step 5: commit** — `feat: MapViewModel route progress from odometer (TDD)`.

### Task 2: `NeonMapView.qml`

**Files:**
- Create: `qml/components/NeonMapView.qml`
- Modify: root `CMakeLists.txt` (QML_FILES)

**Interfaces:**
- Consumes: context properties `MapModel.routeProgress`, `TripComputer.tripDisplay`; tokens `Theme.accentCyan/textSecondary/textPrimary/glassPanelBase/glassPanelBorder/radiusMd/fontMain/textXs/spaceMd/durationGauge`.
- Produces: a self-contained view; design space 400×360 auto-scaled to fit.

- [ ] **Step 1: write the component**

```qml
import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import com.showcase

/*
 * Bản đồ neon cách điệu (Zero-JS). Marker chạy theo quãng đường thật:
 * PathInterpolator.progress bind vào MapModel.routeProgress (C++).
 */
Rectangle {
    id: root
    color: Theme.glassPanelBase
    border.color: Theme.glassPanelBorder
    border.width: 1
    radius: Theme.radiusMd
    clip: true

    // Không gian thiết kế cố định 400×360, scale khít khung
    Item {
        id: mapCanvas
        width: 400
        height: 360
        anchors.centerIn: parent
        scale: Math.min(root.width / width, root.height / height)

        // Lưới phố nền
        Repeater {
            model: 9
            Rectangle {
                required property int index
                x: 40 + index * 40
                width: 1
                height: mapCanvas.height
                color: Theme.textSecondary
                opacity: 0.08
            }
        }
        Repeater {
            model: 8
            Rectangle {
                required property int index
                y: 40 + index * 40
                width: mapCanvas.width
                height: 1
                color: Theme.textSecondary
                opacity: 0.08
            }
        }

        // Phố phụ (nét xám)
        Shape {
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4
            ShapePath {
                strokeColor: Theme.textSecondary
                strokeWidth: 3
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                PathSvg { path: "M 20 120 L 380 120 M 260 20 L 260 340 M 20 220 L 200 220" }
            }
        }

        // Route chính (neon accent — tự đổi màu theo drive mode)
        Shape {
            id: routeShape
            anchors.fill: parent
            layer.enabled: true
            layer.samples: 4
            ShapePath {
                strokeColor: Theme.accentCyan
                strokeWidth: 5
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                PathSvg { path: "M 80 60 L 320 60 Q 340 60 340 80 L 340 160 Q 340 180 320 180 L 220 180 Q 200 180 200 200 L 200 260 Q 200 280 180 280 L 100 280 Q 80 280 80 260 L 80 80 Q 80 60 100 60 Z" }
            }
        }

        // Neon bloom cho route (source là sibling — đúng quy tắc MultiEffect)
        MultiEffect {
            anchors.fill: routeShape
            source: routeShape
            shadowEnabled: true
            shadowColor: Theme.accentCyan
            shadowBlur: 1.0
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }

        // Vị trí + hướng dọc route, điều khiển từ C++
        PathInterpolator {
            id: routeInterp
            progress: MapModel.routeProgress
            path: Path {
                startX: 80; startY: 60
                PathSvg { path: "L 320 60 Q 340 60 340 80 L 340 160 Q 340 180 320 180 L 220 180 Q 200 180 200 200 L 200 260 Q 200 280 180 280 L 100 280 Q 80 280 80 260 L 80 80 Q 80 60 100 60 Z" }
            }
        }

        // Marker mũi tên
        Item {
            id: marker
            x: routeInterp.x
            y: routeInterp.y
            rotation: routeInterp.angle + 90

            Behavior on x { NumberAnimation { duration: Theme.durationGauge } }
            Behavior on y { NumberAnimation { duration: Theme.durationGauge } }

            Shape {
                anchors.centerIn: parent
                width: 18
                height: 22
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    strokeWidth: 0
                    fillColor: Theme.textPrimary
                    startX: 9; startY: 0
                    PathLine { x: 18; y: 22 }
                    PathLine { x: 9; y: 16 }
                    PathLine { x: 0; y: 22 }
                    PathLine { x: 9; y: 0 }
                }
            }
        }
    }

    // Nhãn TRIP góc trên (tái dùng chuỗi format từ C++)
    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Theme.spaceMd
        text: "TRIP " + TripComputer.tripDisplay
        color: Theme.textSecondary
        font.family: Theme.fontMain
        font.pixelSize: Theme.textXs
        font.letterSpacing: 1
    }
}
```

Add `qml/components/NeonMapView.qml` to `QML_FILES`.

- [ ] **Step 2: build + 8s smoke** — no QML errors (view not yet instantiated; compile check via qmlcachegen).
- [ ] **Step 3: commit** — `feat: NeonMapView stylized shapes map with telemetry-driven marker`.

### Task 3: `CenterHub.qml` + DashboardScreen integration

**Files:**
- Create: `qml/components/CenterHub.qml`
- Modify: root `CMakeLists.txt` (QML_FILES), `qml/screens/DashboardScreen.qml`

**Interfaces:**
- Consumes: `MusicPlayer`, `NeonMapView` (Task 2).
- Produces: `CenterHub` item with id target `centerHub` used by DashboardScreen states.

- [ ] **Step 1: write CenterHub**

```qml
import QtQuick
import QtQuick.Controls.Basic
import com.showcase

/*
 * Hub trung tâm lướt được (Car mode): Music ⇄ Map.
 * SwipeView giữ children tĩnh sống mãi — MusicPlayer không bao giờ bị hủy.
 */
Item {
    id: root

    SwipeView {
        id: swipeView
        anchors.fill: parent
        clip: true

        MusicPlayer {}
        NeonMapView {}
    }

    PageIndicator {
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.top: parent.top
        anchors.topMargin: Theme.spaceSm
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.spaceMd
        delegate: Rectangle {
            required property int index
            width: 8
            height: 8
            radius: 4
            color: index === swipeView.currentIndex ? Theme.accentCyan : Theme.trackInactive
            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
        }
    }
}
```

- [ ] **Step 2: DashboardScreen swap** — replace the `MusicPlayer { id: musicPlayer ... }` block inside `centerPanel` with:

```qml
            CenterHub {
                id: centerHub
                anchors.fill: parent
                anchors.margins: 10
                opacity: 1
                visible: opacity > 0
            }
```

(`scooterCard` Loader stays right after it, unchanged.) Then rename state/transition targets: in state `"scooter"` and `"bike"` change `PropertyChanges { target: musicPlayer; opacity: 0 }` → `target: centerHub`; in the Phase-C transition `NumberAnimation` change `targets: [musicPlayer, scooterCard, bottomBar]` → `targets: [centerHub, scooterCard, bottomBar]`.

- [ ] **Step 3: main.cpp wiring** — add `#include "viewmodels/MapViewModel.h"`; after `TripComputerViewModel tripVm;` add `MapViewModel mapVm;`; after the context-property block add `engine.rootContext()->setContextProperty("MapModel", &mapVm);`; and after `tripClock.start();` add:

```cpp
  QObject::connect(&tripVm, &TripComputerViewModel::tripChanged, &mapVm,
                   [&tripVm, &mapVm]() { mapVm.updateDistance(tripVm.odometerKm()); });
```

- [ ] **Step 4: build + smoke** — 0 warnings, 8s run clean; swipe works, marker crawls as the simulator drives.
- [ ] **Step 5: commit** — `feat: CenterHub swipeable Music/Map hub wired into Car mode`.

### Task 4: Verification + docs (NO merge until user confirms)

**Files:**
- Modify: `docs/tasks_board.md`, `docs/journal.md`, `README.md`

- [ ] **Step 1: Zero-JS grep** — `grep -rnE '\bfunction\b|\bif\s*\(|\bfor\s*\(|\bwhile\s*\(|\bswitch\s*\(|\bvar\b|\blet\b|\bconst\b|toFixed|toUpperCase' qml/ --include='*.qml'` → no matches.
- [ ] **Step 2: full build + ctest** — 0 warnings; 31 viewmodel tests + music tests all pass.
- [ ] **Step 3: screenshots** — automated capture covers the Music page (page 0) plus build/test correctness. To capture the Map page without violating Zero-JS, temporarily change `CenterHub.qml`'s SwipeView to `currentIndex: 1` (a declarative literal), rebuild, capture, then revert — a 2-line temporary edit, reverted before the task's commit. The swipe *gesture* itself is verified manually by the user.
- [ ] **Step 4: docs** — tasks_board `## Phase 16` checklist; journal entry (SwipeView static-children rationale, MapModel odometer→progress design, PathInterpolator marker, first Controls import); README: feature bullet + roadmap row 16 + tree entries (`MapViewModel`, `CenterHub.qml`, `NeonMapView.qml`).
- [ ] **Step 5: report to user for manual test** (swipe both directions, marker moves while driving, scooter/bike unchanged, music keeps playing on Map page). After confirmation: squash-merge one commit into `main` + push.

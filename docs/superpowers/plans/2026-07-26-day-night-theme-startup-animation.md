# Day/Night Theme + Startup Animation Implementation Plan

> **Status:** Historical plan retained for the earlier day/night and startup-animation phase.
> The current active focus is the encoder-driven arrow-road scene.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a C++-driven Day/Night theme toggle (Light Glassmorphism day variant) and an automotive boot sequence (telltale self-test → gauge sweep → content fade-in) to the CyberDash cluster.

**Architecture:** A new `ThemeViewModel` (context property `ThemeController`) owns the `isNight` flag and the boot timeline (`QSequentialAnimationGroup` driving `bootProgress` 0→1→0 and `bootStage` 0→1→2). `Theme.qml` color tokens become ternaries on `ThemeController.isNight` with `Behavior { ColorAnimation }` so the whole app cross-fades. QML uses only declarative ternary bindings — Zero JS.

**Tech Stack:** Qt 6.8 (Core, Qml, Quick, Test), CMake + Ninja, QtTest.

**Spec:** `docs/superpowers/specs/2026-07-26-day-night-theme-startup-animation-design.md`

## Global Constraints

- Zero JS in QML: no `function`, `if`, `for`, `while`, `switch`, `var`, `let`, `const` in any `.qml`. Ternary property bindings and `Behavior`/`NumberAnimation`/`ColorAnimation` are allowed.
- All new behavior lives in C++ (`ThemeViewModel`); QML unchanged when swapping Simulator ↔ Serial.
- C++17, `-Wall -Wextra -Wconversion -Wsign-conversion` must stay clean (use `static_cast` where needed).
- Build dir: `build/` (`cmake -B build && cmake --build build`), tests via `ctest --test-dir build`.
- **Commit policy for this run (user override of CLAUDE.md vibe-coding rule): do NOT commit/push per task. One commit at the very end, only after the user manually tests and confirms.** Per-task "Commit" steps are intentionally omitted.
- The Double Arch bezel outline must look identical in both themes (new constant `bezelStroke` token); only the screen face and content change.

---

### Task 1: `ThemeViewModel` (TDD)

**Files:**
- Create: `src/viewmodels/ThemeViewModel.h`
- Create: `src/viewmodels/ThemeViewModel.cpp`
- Modify: `tests/main.cpp` (add include + test slots)
- Modify: `tests/CMakeLists.txt` (add source to `tst_viewmodels`)

**Interfaces:**
- Consumes: nothing (self-contained QObject).
- Produces (used by Tasks 2–5):
  - `explicit ThemeViewModel(int sweepLegDurationMs = 900, QObject *parent = nullptr)`
  - `bool isNight() const` — default `true`; `Q_INVOKABLE void toggleTheme()`
  - `int bootStage() const` — 0 pre-boot, 1 sweeping, 2 done
  - `bool isBooting() const` — `bootStage() < 2`
  - `qreal bootProgress() const` — 0→1→0 during stage 1
  - `void startBootSequence()` — idempotent (second call is a no-op)
  - Signals: `isNightChanged()`, `bootStageChanged()`, `bootProgressChanged()`

- [ ] **Step 1: Write the failing tests**

Append to `tests/main.cpp` (inside `class TestViewModels`, after `testUpdateTelemetry`), and add `#include "viewmodels/ThemeViewModel.h"` at the top next to the existing include:

```cpp
    // --- ThemeViewModel (Phase 13) ---

    void testThemeDefaultIsNight() {
        ThemeViewModel theme;
        QCOMPARE(theme.isNight(), true);
        QCOMPARE(theme.bootStage(), 0);
        QCOMPARE(theme.isBooting(), true);
        QCOMPARE(theme.bootProgress(), 0.0);
    }

    void testToggleTheme() {
        ThemeViewModel theme;
        QSignalSpy spy(&theme, &ThemeViewModel::isNightChanged);

        theme.toggleTheme();
        QCOMPARE(theme.isNight(), false);
        QCOMPARE(spy.count(), 1);

        theme.toggleTheme();
        QCOMPARE(theme.isNight(), true);
        QCOMPARE(spy.count(), 2);
    }

    void testBootSequence() {
        ThemeViewModel theme(10); // 10 ms per sweep leg -> full boot ~20 ms
        QSignalSpy stageSpy(&theme, &ThemeViewModel::bootStageChanged);

        qreal maxProgress = 0.0;
        connect(&theme, &ThemeViewModel::bootProgressChanged, this, [&]() {
            maxProgress = qMax(maxProgress, theme.bootProgress());
        });

        theme.startBootSequence();
        QCOMPARE(theme.bootStage(), 1);

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(theme.isBooting(), false);
        QVERIFY(maxProgress > 0.9);
        QCOMPARE(theme.bootProgress(), 0.0);
        QCOMPARE(stageSpy.count(), 2); // 0->1 and 1->2
    }

    void testBootSequenceIsIdempotent() {
        ThemeViewModel theme(10);
        QSignalSpy stageSpy(&theme, &ThemeViewModel::bootStageChanged);

        theme.startBootSequence();
        theme.startBootSequence(); // must be a no-op

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(stageSpy.count(), 2);
    }

    void testToggleDuringBoot() {
        ThemeViewModel theme(50);
        theme.startBootSequence();
        QCOMPARE(theme.bootStage(), 1);

        theme.toggleTheme(); // theme and boot are independent
        QCOMPARE(theme.isNight(), false);

        QTRY_COMPARE_WITH_TIMEOUT(theme.bootStage(), 2, 2000);
        QCOMPARE(theme.isNight(), false);
    }
```

- [ ] **Step 2: Add the new source to the test target and run to verify FAIL**

In `tests/CMakeLists.txt`, extend the first executable:

```cmake
qt_add_executable(tst_viewmodels
    main.cpp
    ../src/viewmodels/VehicleStatusViewModel.cpp
    ../src/viewmodels/ThemeViewModel.cpp
)
```

Run: `cmake -B build && cmake --build build 2>&1 | tail -20`
Expected: **compile FAILURE** — `viewmodels/ThemeViewModel.h: No such file or directory`. That is the TDD "red".

- [ ] **Step 3: Write the header**

`src/viewmodels/ThemeViewModel.h`:

```cpp
#pragma once

#include <QObject>
#include <QPointer>

class QSequentialAnimationGroup;

class ThemeViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isNight READ isNight NOTIFY isNightChanged)
    Q_PROPERTY(int bootStage READ bootStage NOTIFY bootStageChanged)
    Q_PROPERTY(bool isBooting READ isBooting NOTIFY bootStageChanged)
    Q_PROPERTY(qreal bootProgress READ bootProgress NOTIFY bootProgressChanged)

public:
    explicit ThemeViewModel(int sweepLegDurationMs = 900, QObject *parent = nullptr);

    bool isNight() const;
    int bootStage() const;
    bool isBooting() const;
    qreal bootProgress() const;

    Q_INVOKABLE void toggleTheme();
    void startBootSequence();

signals:
    void isNightChanged();
    void bootStageChanged();
    void bootProgressChanged();

private:
    void setBootStage(int stage);
    void setBootProgress(qreal progress);

    bool m_isNight = true;
    int m_bootStage = 0;
    qreal m_bootProgress = 0.0;
    int m_sweepLegDurationMs;
    QPointer<QSequentialAnimationGroup> m_bootTimeline;
};
```

- [ ] **Step 4: Write the implementation**

`src/viewmodels/ThemeViewModel.cpp`:

```cpp
#include "ThemeViewModel.h"

#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>

ThemeViewModel::ThemeViewModel(int sweepLegDurationMs, QObject *parent)
    : QObject(parent), m_sweepLegDurationMs(sweepLegDurationMs)
{
}

bool ThemeViewModel::isNight() const { return m_isNight; }
int ThemeViewModel::bootStage() const { return m_bootStage; }
bool ThemeViewModel::isBooting() const { return m_bootStage < 2; }
qreal ThemeViewModel::bootProgress() const { return m_bootProgress; }

void ThemeViewModel::toggleTheme()
{
    m_isNight = !m_isNight;
    emit isNightChanged();
}

void ThemeViewModel::startBootSequence()
{
    if (m_bootTimeline) {
        return; // already started (idempotent)
    }

    auto *timeline = new QSequentialAnimationGroup(this);
    m_bootTimeline = timeline;

    const auto addLeg = [this, timeline](qreal from, qreal to) {
        auto *leg = new QVariantAnimation(timeline);
        leg->setStartValue(from);
        leg->setEndValue(to);
        leg->setDuration(m_sweepLegDurationMs);
        leg->setEasingCurve(QEasingCurve::InOutQuad);
        connect(leg, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) { setBootProgress(value.toReal()); });
        timeline->addAnimation(leg);
    };

    addLeg(0.0, 1.0);
    addLeg(1.0, 0.0);

    connect(timeline, &QSequentialAnimationGroup::finished, this,
            [this]() { setBootStage(2); });

    setBootStage(1);
    timeline->start();
}

void ThemeViewModel::setBootStage(int stage)
{
    if (m_bootStage == stage) {
        return;
    }
    m_bootStage = stage;
    emit bootStageChanged();
}

void ThemeViewModel::setBootProgress(qreal progress)
{
    if (qFuzzyCompare(m_bootProgress, progress)) {
        return;
    }
    m_bootProgress = progress;
    emit bootProgressChanged();
}
```

Note: `qFuzzyCompare(0.0, x)` is unreliable for zero, but here both endpoints are set exactly by `QVariantAnimation` (start 0.0 → many intermediate values → end 0.0), and the guard only suppresses duplicate emissions — correctness of the final value is untouched.

- [ ] **Step 5: Build and run tests to verify PASS**

Run: `cmake --build build && ctest --test-dir build --output-on-failure -R tst_viewmodels`
Expected: PASS, including the 5 new theme tests and all pre-existing ones.

---

### Task 2: Register `ThemeController` in the app

**Files:**
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt` (root — add sources to the QML module)

**Interfaces:**
- Consumes: `ThemeViewModel` from Task 1.
- Produces: QML context property `ThemeController` (used by Tasks 3–5) and the boot kick-off `startBootSequence()` after engine load.

- [ ] **Step 1: Add sources to the QML module target**

In root `CMakeLists.txt`, inside `qt_add_qml_module(... SOURCES ...)`, extend:

```cmake
    SOURCES
        src/viewmodels/VehicleStatusViewModel.h
        src/viewmodels/VehicleStatusViewModel.cpp
        src/viewmodels/MusicPlayerViewModel.h
        src/viewmodels/MusicPlayerViewModel.cpp
        src/viewmodels/ThemeViewModel.h
        src/viewmodels/ThemeViewModel.cpp
```

- [ ] **Step 2: Wire it in `main.cpp`**

In `src/main.cpp`:
1. Add `#include "viewmodels/ThemeViewModel.h"` next to the other viewmodel includes.
2. Next to `MusicPlayerViewModel musicVm;` add and expose:

```cpp
  ThemeViewModel themeVm;
  engine.rootContext()->setContextProperty("VehicleStatus", &vm);
  engine.rootContext()->setContextProperty("MusicViewModel", &musicVm);
  engine.rootContext()->setContextProperty("ThemeController", &themeVm);
```

3. Immediately after `engine.loadFromModule("com.showcase", "Main");` add:

```cpp
  themeVm.startBootSequence();
```

- [ ] **Step 3: Build clean**

Run: `cmake --build build 2>&1 | tail -5`
Expected: build succeeds, zero warnings from the new files.

---

### Task 3: Day theme tokens in `Theme.qml` + constant bezel

**Files:**
- Modify: `qml/Theme.qml`
- Modify: `qml/Main.qml` (bezel stroke uses new constant token)

**Interfaces:**
- Consumes: `ThemeController.isNight` (Task 2).
- Produces: themed tokens (same names as today, so no other file changes) plus new constant `readonly property color bezelStroke`.

- [ ] **Step 1: Rewrite the color section of `Theme.qml`**

Replace the color block (keep timings/radii/typography/spacing/geometry sections untouched). Themed tokens lose `readonly` (a `Behavior` cannot attach to a readonly property); constants keep it. `durationTheme: 600` is added to the timing section.

```qml
pragma Singleton
import QtQuick

QtObject {
    // Cyberpunk Color Palette — Night (default) / Day (Light Glassmorphism)
    property color backgroundDeepSpace: ThemeController.isNight ? "#0B0C10" : "#DDE7EE"
    property color accentCyan: ThemeController.isNight ? "#66FCF1" : "#00857C"
    readonly property color warningRed: "#FF3B30"
    property color textPrimary: ThemeController.isNight ? "#FFFFFF" : "#1A2530"
    property color textSecondary: ThemeController.isNight ? "#C5C6C7" : "#4A5A68"
    property color textOnAccent: ThemeController.isNight ? "#0B0C10" : "#F2F7FA"

    property color glassPanelBase: ThemeController.isNight ? "#40151D26" : "#8CE8F0F8"
    property color glassPanelBorder: ThemeController.isNight ? "#802A3B4C" : "#80FFFFFF"
    property color trackInactive: ThemeController.isNight ? "#40FFFFFF" : "#401A2530"

    property color tickLitMajor: ThemeController.isNight ? "#FFB3CC" : "#D81B60"
    property color tickLitMinor: ThemeController.isNight ? "#FFFFFF" : "#0E8F88"
    property color tickDimMajor: ThemeController.isNight ? "#2A3B4C" : "#B8C6D1"
    property color tickDimMinor: ThemeController.isNight ? "#151D26" : "#D3DDE5"

    readonly property color coverFallback1: "#FF0055"
    readonly property color coverFallback2: "#4A00E0"
    readonly property color glassEdge: Qt.rgba(1.0, 1.0, 1.0, 0.15)
    // Physical bezel outline — identical in both themes (was derived from textSecondary)
    readonly property color bezelStroke: "#80C5C6C7"

    // Smooth cross-fade when the theme flips (single place for the whole app)
    Behavior on backgroundDeepSpace { ColorAnimation { duration: durationTheme } }
    Behavior on accentCyan { ColorAnimation { duration: durationTheme } }
    Behavior on textPrimary { ColorAnimation { duration: durationTheme } }
    Behavior on textSecondary { ColorAnimation { duration: durationTheme } }
    Behavior on textOnAccent { ColorAnimation { duration: durationTheme } }
    Behavior on glassPanelBase { ColorAnimation { duration: durationTheme } }
    Behavior on glassPanelBorder { ColorAnimation { duration: durationTheme } }
    Behavior on trackInactive { ColorAnimation { duration: durationTheme } }
    Behavior on tickLitMajor { ColorAnimation { duration: durationTheme } }
    Behavior on tickLitMinor { ColorAnimation { duration: durationTheme } }
    Behavior on tickDimMajor { ColorAnimation { duration: durationTheme } }
    Behavior on tickDimMinor { ColorAnimation { duration: durationTheme } }
```

And in the Animation Timings section add:

```qml
    readonly property int durationTheme: 600
```

- [ ] **Step 2: Keep the bezel constant in `Main.qml`**

In `qml/Main.qml`, the first `ShapePath` currently derives its stroke from a themed token:

```qml
strokeColor: Qt.rgba(Theme.textSecondary.r, Theme.textSecondary.g, Theme.textSecondary.b, 0.5)
```

Replace with:

```qml
strokeColor: Theme.bezelStroke
```

(`fillColor: Theme.backgroundDeepSpace` stays — the fill is the screen face and SHOULD theme; the outline and `glassEdge` inset stroke stay constant.)

- [ ] **Step 3: Build + quick smoke**

Run: `cmake --build build && timeout 8 ./build/QtStmAutomotiveSimulator; test $? -eq 124 && echo SMOKE-OK`
Expected: `SMOKE-OK`, no QML errors/warnings about `ThemeController` in stderr. Visual: unchanged night look.

---

### Task 4: Theme toggle button (sun/moon)

**Files:**
- Create: `resources/icons/theme-day.svg`
- Create: `resources/icons/theme-night.svg`
- Modify: `CMakeLists.txt` (RESOURCES)
- Modify: `qml/screens/DashboardScreen.qml`

**Interfaces:**
- Consumes: `ThemeController.isNight`, `ThemeController.toggleTheme()` (Task 2); existing `NeonIconButton` (`source`, `sourceSize`, `clicked()`).
- Produces: user-facing toggle; no new API.

- [ ] **Step 1: Create the stroke-style icons (match existing 24×24, stroke #fff, width 2)**

`resources/icons/theme-day.svg` (sun):

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <circle cx="12" cy="12" r="4"/>
  <line x1="12" y1="1.5" x2="12" y2="4"/>
  <line x1="12" y1="20" x2="12" y2="22.5"/>
  <line x1="1.5" y1="12" x2="4" y2="12"/>
  <line x1="20" y1="12" x2="22.5" y2="12"/>
  <line x1="4.6" y1="4.6" x2="6.4" y2="6.4"/>
  <line x1="17.6" y1="17.6" x2="19.4" y2="19.4"/>
  <line x1="4.6" y1="19.4" x2="6.4" y2="17.6"/>
  <line x1="17.6" y1="6.4" x2="19.4" y2="4.6"/>
</svg>
```

`resources/icons/theme-night.svg` (moon):

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
  <path d="M 20 14.5 A 8.5 8.5 0 1 1 9.5 4 A 7 7 0 0 0 20 14.5 Z"/>
</svg>
```

- [ ] **Step 2: Register in CMake**

In root `CMakeLists.txt` under `RESOURCES`, after `resources/icons/temperature-high.svg` add:

```cmake
        resources/icons/theme-day.svg
        resources/icons/theme-night.svg
```

- [ ] **Step 3: Add the button to `DashboardScreen.qml`**

After the `RowLayout { id: topBar ... }` block, insert as a sibling (icon shows the mode you will switch TO):

```qml
    NeonIconButton {
        width: 28
        height: 28
        anchors.verticalCenter: topBar.verticalCenter
        anchors.left: topBar.right
        anchors.leftMargin: Theme.spaceXXl
        source: ThemeController.isNight
                ? "qrc:/qt/qml/com/showcase/resources/icons/theme-day.svg"
                : "qrc:/qt/qml/com/showcase/resources/icons/theme-night.svg"
        sourceSize: Qt.size(28, 28)
        defaultColor: Theme.textSecondary
        onClicked: ThemeController.toggleTheme()
    }
```

- [ ] **Step 4: Build + smoke + manual sanity**

Run: `cmake --build build && timeout 8 ./build/QtStmAutomotiveSimulator; test $? -eq 124 && echo SMOKE-OK`
Expected: `SMOKE-OK`, no QML errors. (Full visual toggle check happens in Task 6 / user review.)

---

### Task 5: Boot choreography bindings

**Files:**
- Modify: `qml/components/NeonTickGauge.qml`
- Modify: `qml/screens/DashboardScreen.qml`

**Interfaces:**
- Consumes: `ThemeController.isBooting`, `ThemeController.bootStage`, `ThemeController.bootProgress` (Tasks 1–2).
- Produces: visual boot sequence; no new API.

- [ ] **Step 1: Gauge sweep in `NeonTickGauge.qml`**

Replace the `displayedValue` block (lines 25–29):

```qml
    // Giá trị hiển thị chạy mượt mà (trailing effect).
    // Khi boot: hiển thị sweep 0→max→0 do C++ điều khiển (Behavior tắt để bám sát timeline).
    property real displayedValue: ThemeController.isBooting
                                  ? ThemeController.bootProgress * maxValue
                                  : value
    Behavior on displayedValue {
        enabled: !ThemeController.isBooting
        NumberAnimation { duration: Theme.durationGauge; easing.type: Easing.OutQuad }
    }
```

- [ ] **Step 2: Telltale self-test + content fade in `DashboardScreen.qml`**

1. The 3 `NeonIcon` telltales: prefix each binding with the boot state (all lit while booting). Example for the warning icon — apply the same pattern to battery (`vm.battery < 20`) and temperature (`vm.temperature > 85`):

```qml
        NeonIcon {
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: "qrc:/qt/qml/com/showcase/resources/icons/warning-triangle.svg"
            sourceSize: Qt.size(28, 28)
            colorizationColor: ThemeController.isBooting ? Theme.warningRed
                               : (vm.isWarning ? Theme.warningRed : Theme.textSecondary)
            opacity: ThemeController.isBooting ? 1.0 : (vm.isWarning ? 1.0 : 0.2)
        }
```

(`NeonIcon` already has `Behavior on opacity`, so the self-test → live transition fades automatically.)

2. Center panel fade-in — add to `Item { id: centerPanel ... }`:

```qml
            opacity: ThemeController.bootStage < 2 ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: Theme.durationSlow } }
```

3. Bottom bar fade-in — add `id: bottomBar` plus the same two lines to the bottom `RowLayout`:

```qml
    RowLayout {
        id: bottomBar
        opacity: ThemeController.bootStage < 2 ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: Theme.durationSlow } }
```

- [ ] **Step 3: Build + smoke run with boot visible**

Run: `cmake --build build && timeout 8 ./build/QtStmAutomotiveSimulator; test $? -eq 124 && echo SMOKE-OK`
Expected: `SMOKE-OK`, no QML errors; on launch the gauges sweep up/down (~1.8 s), telltales start lit then settle, center + bottom content fades in.

---

### Task 6: Final verification + docs (NO commit until user confirms)

**Files:**
- Modify: `docs/tasks_board.md` (add Phase 13 section)
- Modify: `docs/journal.md` (entry)

**Interfaces:**
- Consumes: everything above.
- Produces: verified feature + updated progress docs.

- [ ] **Step 1: Zero-JS pre-flight grep**

Run: `grep -rnE '\bfunction\b|\bif\s*\(|\bfor\s*\(|\bwhile\s*\(|\bswitch\s*\(|\bvar\b|\blet\b|\bconst\b' qml/ --include='*.qml'`
Expected: no matches (ternaries and Behaviors are fine and won't match).

- [ ] **Step 2: Full clean-build + full test suite**

Run: `cmake -B build && cmake --build build 2>&1 | tail -5 && ctest --test-dir build --output-on-failure`
Expected: build clean (no warnings), **all tests pass** (tst_viewmodels incl. 5 new theme tests + tst_music_playback).

- [ ] **Step 3: 8 s smoke run**

Run: `timeout 8 ./build/QtStmAutomotiveSimulator 2>&1; test $? -eq 124 && echo SMOKE-OK`
Expected: `SMOKE-OK` and empty/clean stderr (no `qrc` missing-file or binding-loop warnings).

- [ ] **Step 4: Update progress docs**

Append to `docs/tasks_board.md`:

```markdown
## Phase 13: Day/Night Theme + Startup Animation
- [x] Implement `ThemeViewModel` (TDD): `isNight` toggle + boot timeline (`bootStage`, `bootProgress`) driven by `QSequentialAnimationGroup`.
- [x] Register `ThemeController` context property and boot kick-off in `main.cpp`.
- [x] Theme.qml: Day (Light Glassmorphism) token set via ternaries + centralized `Behavior { ColorAnimation }`; constant `bezelStroke` keeps the Double Arch identical in both themes.
- [x] Sun/moon `NeonIconButton` toggle in the top bar (new stroke-style SVG icons).
- [x] Boot choreography: telltale self-test, gauge sweep 0→max→0, sequential center/bottom fade-in — all declarative ternary bindings (Zero JS).
- [x] Verify: clean build, all ctest pass, 8s smoke run clean.
```

Add a matching short entry to `docs/journal.md` following its existing format (read it first).

- [ ] **Step 5: Notify the user for manual testing — STOP before committing**

Report: what was built, how to run it (`./build/QtStmAutomotiveSimulator`), what to check (boot sweep on launch, sun/moon toggle cross-fade, bezel unchanged). **Only after the user confirms**, make a single commit of all changes:

```bash
git add -A
git commit -m "feat: Day/Night theme toggle + automotive startup animation (Phase 13)

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
git push
```

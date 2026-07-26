# Design Spec: Day/Night Theme + Startup Animation

> **Date**: 2026-07-26
> **Status**: Approved by user (brainstorming session)
> **Phase**: 13 (next after Functional Telltale Bar)

## 1. Goal

Two related UI upgrades, both driven by a single new C++ ViewModel:

1. **Day/Night theme switching** — a top-bar button toggles the entire cluster between
   the current dark Neon Cyberpunk shell (Night) and a new Light Glassmorphism
   variant (Day).
2. **Startup animation** — on application launch the cluster performs an automotive
   boot sequence: telltale self-test, gauge tick sweep 0 → max → 0, then sequential
   fade-in of the central content.

Both must respect the project's global directives: Zero JS in QML, MVVM with
`Q_PROPERTY`, QML unchanged when swapping Simulator ↔ Serial.

## 2. Architecture (chosen: Approach A — dedicated C++ ViewModel)

### 2.1 New class: `ThemeViewModel`

Location: `src/viewmodels/ThemeViewModel.h` / `.cpp`. Registered as context
property `ThemeController` in `main.cpp`, same pattern as the existing ViewModels.

Public API:

| Member | Kind | Behavior |
|---|---|---|
| `isNight` | `Q_PROPERTY(bool, READ + NOTIFY)` | Theme flag. Default `true` (preserves current aesthetic on first launch). |
| `toggleTheme()` | `Q_INVOKABLE` | Flips `isNight`, emits `isNightChanged`. Allowed at any time, including during boot. |
| `isBooting` | `Q_PROPERTY(bool, READ + NOTIFY)` | `true` from construction until the boot timeline ends. Derived: `bootStage < 2`. |
| `bootStage` | `Q_PROPERTY(int, READ + NOTIFY)` | `0` = pre-sweep (content hidden, telltales self-test), `1` = sweeping, `2` = done (content revealed, telltales live). |
| `bootProgress` | `Q_PROPERTY(qreal, READ + NOTIFY)` | Sweep position 0 → 1 → 0 during stage 1. |
| `startBootSequence()` | public method | Starts the boot timeline. Called from `main.cpp` after engine load; callable directly in tests. |

Boot timeline is a C++ `QSequentialAnimationGroup`:
`QVariantAnimation` 0 → 1 (900 ms, `InOutQuad`) then 1 → 0 (900 ms, `InOutQuad`)
driving `bootProgress`; stage transitions 0 → 1 at start and 1 → 2 at finish.
Total ≈ 1.8 s.

**Testability**: constructor takes an optional per-leg duration parameter
(default 900 ms) so unit tests can run the full timeline in milliseconds and
assert with `QSignalSpy`.

### 2.2 Data flow

```text
NeonIconButton (sun/moon, top bar)
    └─ onClicked → ThemeController.toggleTheme()      [Q_INVOKABLE]
         └─ isNightChanged
              └─ Theme.qml token ternaries re-evaluate
                   └─ every component re-binds automatically
```

Only `Theme.qml` (tokens) and the boot-aware bindings in `DashboardScreen.qml` /
`NeonTickGauge.qml` reference `ThemeController`. All other components keep reading
`Theme.*` tokens exclusively.

## 3. Day theme — Light Glassmorphism

Only **color** tokens change; geometry, radii, durations, typography are shared.

- Every color token in `Theme.qml` becomes
  `ThemeController.isNight ? <night value> : <day value>`.
- Glass panels: dark `#2C353F` tinted base → frosted blue-white
  (white-blue rgba at ~55% opacity), same diagonal gradient direction.
- Text and unlit ticks: light tones → deep navy `#1A2530` for contrast.
- Neon accents: same hues (cyan/magenta family), reduced bloom/glow intensity so
  they read cleanly on the light background.
- **The Double Arch bezel stays dark in both themes** — it represents physical
  hardware; only the "screen" content changes.
- Smooth transition: `Behavior on <token> { ColorAnimation }` declared **inside
  `Theme.qml`** on each color token — one place, the whole app animates; no
  per-component changes and no JS.

## 4. Startup choreography

| Stage | Time | Visual |
|---|---|---|
| 0 | t = 0 | Central glass panel + music player at opacity 0. All 3 telltales lit (automotive self-test). Gauges at 0. |
| 1 | 0 → 1.8 s | `bootProgress` sweeps 0 → 1 → 0. `NeonTickGauge` displays `isBooting ? bootProgress : value / max` (pure ternary binding). |
| 2 | ≥ 1.8 s | Central content fades in via existing `Behavior on opacity`; telltales return to live bindings (`isWarning`, battery < 20, temperature > 85). |

QML expresses all of this with declarative ternaries on `bootStage` /
`isBooting` plus `Behavior` animations — no `SequentialAnimation` scripting and
no JS blocks in QML. The C++ animation group is the single timeline owner.

## 5. Edge cases

- **Toggle during boot**: theme and boot are independent states in the same
  ViewModel; no locking, both work simultaneously.
- **Telemetry during boot**: `VehicleStatusViewModel` keeps ingesting
  Simulator/Serial data normally; gauges merely *display* the sweep until stage 2.
  No data loss, and the QML swap-invariance golden check holds because boot state
  lives entirely outside the telemetry path.
- **Deterministic tests**: injected short durations remove wall-clock dependence.

## 6. Testing plan (TDD — tests written first)

In `tests/main.cpp` (QtTest):

1. `testThemeDefaultIsNight` — fresh ViewModel has `isNight == true`.
2. `testToggleTheme` — value flips and `isNightChanged` fires exactly once.
3. `testBootSequence` — with ~10 ms legs: stages 0 → 1 → 2 in order,
   `bootProgress` reaches 1 then returns to 0, ends with `isBooting == false`.
4. `testToggleDuringBoot` — toggling mid-sweep still flips `isNight`.
5. Existing 8 s smoke run: no QML errors, boot animation plays, toggle works.

## 7. File impact

| File | Change |
|---|---|
| `src/viewmodels/ThemeViewModel.h/.cpp` | **New** — theme flag + boot timeline. |
| `tests/main.cpp` | New test cases (written before implementation). |
| `src/main.cpp` | Register `ThemeController`, call `startBootSequence()` after load. |
| `CMakeLists.txt` | Add new sources. |
| `qml/Theme.qml` | Color tokens → ternaries + `Behavior` color animations. |
| `qml/screens/DashboardScreen.qml` | Sun/moon `NeonIconButton` in top bar; opacity bindings on `bootStage`; telltale self-test binding. |
| `qml/components/NeonTickGauge.qml` | Displayed fraction ternary for boot sweep. |

## 8. Out of scope (YAGNI)

- Automatic switching by clock time or ambient-light telemetry (button-only for now;
  the ViewModel API does not preclude adding these later).
- Theme persistence across restarts.
- Logo/splash boot screen and bezel draw-in effects.
